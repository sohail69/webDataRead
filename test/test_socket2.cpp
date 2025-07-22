// main.cpp

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <openssl/err.h>
#include <openssl/sha.h>
#include <openssl/ssl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <random>
#include <sstream>
#include <iomanip>

// Constants
const std::string HOST = "stream.data.alpaca.markets";
const std::string PORT = "443";
const std::string PATH = "/v1beta3/crypto/us";
const int MAX_MESSAGES = 10;

// Replace these with your Alpaca API credentials
const std::string API_KEY = "YOUR_API_KEY";
const std::string API_SECRET = "YOUR_API_SECRET";




/**
 * Helper function to send data over SSL connection.
 */
bool ssl_send_all(SSL* ssl, const std::string& data) {
    int total_sent = 0;
    int data_len = data.length();
    const char* buf = data.c_str();

    while (total_sent < data_len) {
        int sent = SSL_write(ssl, buf + total_sent, data_len - total_sent);
        if (sent <= 0) {
            std::cerr << "SSL_write failed." << std::endl;
            return false;
        }
        total_sent += sent;
    }
    return true;
}

/**
 * Helper function to receive data over SSL connection.
 */
std::string ssl_receive(SSL* ssl) {
    const int buffer_size = 4096;
    char buffer[buffer_size];
    std::string received;
    int bytes;

    while ((bytes = SSL_read(ssl, buffer, buffer_size)) > 0) {
        received.append(buffer, bytes);
        // WebSocket handshake response ends with double CRLF
        if (received.find("\r\n\r\n") != std::string::npos) {
            break;
        }
    }
    return received;
}

/**
 * Parses the Sec-WebSocket-Accept from the server's handshake response.
 */
std::string parse_websocket_accept(const std::string& response, const std::string& websocket_key) {
    std::string accept_key;
    std::string key_guid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    size_t pos = response.find("Sec-WebSocket-Accept:");
    if (pos != std::string::npos) {
        pos += strlen("Sec-WebSocket-Accept:");
        size_t end = response.find("\r\n", pos);
        if (end != std::string::npos) {
            accept_key = response.substr(pos, end - pos);
            // Trim whitespace
            accept_key.erase(std::remove_if(accept_key.begin(), accept_key.end(), ::isspace), accept_key.end());
        }
    }

    // Compute the expected accept key
    std::string combined = websocket_key + key_guid;
    std::string sha1 = sha1_hash(combined);
    // Base64 encode the SHA1 hash
    // Since base64_encode expects unsigned char*, we need to handle it appropriately
    return base64_encode(reinterpret_cast<const unsigned char*>(sha1.c_str()), sha1.length());
}

/**
 * Constructs the WebSocket handshake HTTP request.
 */
std::string construct_handshake_request(const std::string& host, const std::string& path, const std::string& websocket_key) {
    std::stringstream ss;
    ss << "GET " << path << " HTTP/1.1\r\n";
    ss << "Host: " << host << "\r\n";
    ss << "Upgrade: websocket\r\n";
    ss << "Connection: Upgrade\r\n";
    ss << "Sec-WebSocket-Key: " << websocket_key << "\r\n";
    ss << "Sec-WebSocket-Version: 13\r\n";
    ss << "\r\n";
    return ss.str();
}

/**
 * Parses the HTTP response code from the handshake response.
 */
int parse_http_response_code(const std::string& response) {
    if (response.substr(0, 12) == "HTTP/1.1 101") {
        return 101; // Switching Protocols
    }
    return -1;
}

/**
 * Helper function to create a WebSocket frame.
 * Supports only text frames without masking (for server-to-client communication).
 */
std::vector<unsigned char> create_websocket_frame(const std::string& message) {
    std::vector<unsigned char> frame;
    frame.push_back(0x81); // FIN bit set and text frame

    if (message.length() <= 125) {
        frame.push_back(static_cast<unsigned char>(message.length()));
    } else if (message.length() <= 65535) {
        frame.push_back(126);
        uint16_t len = htons(static_cast<uint16_t>(message.length()));
        frame.push_back((len >> 8) & 0xFF);
        frame.push_back(len & 0xFF);
    } else {
        frame.push_back(127);
        uint64_t len = htobe64(static_cast<uint64_t>(message.length()));
        for (int i = 0; i < 8; ++i) {
            frame.push_back((len >> (56 - 8 * i)) & 0xFF);
        }
    }

    // No masking for server-to-client frames
    frame.insert(frame.end(), message.begin(), message.end());
    return frame;
}

/**
 * Helper function to parse WebSocket frames.
 * This function assumes that the entire frame has been received.
 * It returns the payload as a string.
 */
std::string parse_websocket_frame(const std::vector<unsigned char>& data) {
    if (data.size() < 2) {
        return "";
    }

    unsigned char fin = data[0] & 0x80;
    unsigned char opcode = data[0] & 0x0F;
    unsigned char mask = data[1] & 0x80;
    uint64_t payload_length = data[1] & 0x7F;
    size_t index = 2;

    if (payload_length == 126) {
        if (data.size() < 4) return "";
        payload_length = (data[2] << 8) | data[3];
        index += 2;
    } else if (payload_length == 127) {
        if (data.size() < 10) return "";
        payload_length = 0;
        for (int i = 0; i < 8; ++i) {
            payload_length = (payload_length << 8) | data[2 + i];
        }
        index += 8;
    }

    unsigned char masking_key[4];
    if (mask) {
        if (data.size() < index + 4) return "";
        for (int i = 0; i < 4; ++i) {
            masking_key[i] = data[index + i];
        }
        index += 4;
    }

    if (data.size() < index + payload_length) return "";

    std::string payload;
    payload.reserve(payload_length);
    for (size_t i = 0; i < payload_length; ++i) {
        char byte = data[index + i];
        if (mask) {
            byte ^= masking_key[i % 4];
        }
        payload += byte;
    }

    return payload;
}

int main() {




    // Receive handshake response
    std::string handshake_response = ssl_receive(ssl);
    // std::cout << "Handshake Response:\n" << handshake_response << std::endl;

    // Validate handshake response
    int response_code = parse_http_response_code(handshake_response);
    if (response_code != 101) {
        std::cerr << "Failed to perform WebSocket handshake. HTTP response code: " << response_code << std::endl;
        SSL_shutdown(ssl);
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        close(sockfd);
        return EXIT_FAILURE;
    }

    // Validate Sec-WebSocket-Accept
    std::string expected_accept = parse_websocket_accept(handshake_response, websocket_key);
    // Extract the actual accept key from the response
    size_t pos = handshake_response.find("Sec-WebSocket-Accept:");
    std::string actual_accept;
    if (pos != std::string::npos) {
        pos += strlen("Sec-WebSocket-Accept:");
        size_t end = handshake_response.find("\r\n", pos);
        if (end != std::string::npos) {
            actual_accept = handshake_response.substr(pos, end - pos);
            // Trim whitespace
            actual_accept.erase(std::remove_if(actual_accept.begin(), actual_accept.end(), ::isspace), actual_accept.end());
        }
    }

    if (expected_accept != actual_accept) {
        std::cerr << "WebSocket handshake failed: Invalid Sec-WebSocket-Accept." << std::endl;
        SSL_shutdown(ssl);
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        close(sockfd);
        return EXIT_FAILURE;
    }

    std::cout << "WebSocket handshake successful." << std::endl;

    // Send authentication message
    std::string auth_message = "{\"action\":\"auth\",\"key\":\"" + API_KEY + "\",\"secret\":\"" + API_SECRET + "\"}";
    std::vector<unsigned char> auth_frame = create_websocket_frame(auth_message);
    std::string auth_frame_str(auth_frame.begin(), auth_frame.end());

    if (!ssl_send_all(ssl, auth_frame_str)) {
        std::cerr << "Failed to send authentication message." << std::endl;
        SSL_shutdown(ssl);
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        close(sockfd);
        return EXIT_FAILURE;
    }

    std::cout << "Authentication message sent." << std::endl;

    // Communication loop: receive messages and print them
    int messages_received = 0;
    while (messages_received < MAX_MESSAGES) {
        const int buffer_size = 4096;
        std::vector<unsigned char> buffer(buffer_size);
        int bytes = SSL_read(ssl, buffer.data(), buffer_size);
        if (bytes <= 0) {
            std::cerr << "SSL_read failed or connection closed." << std::endl;
            break;
        }

        buffer.resize(bytes);
        std::string payload = parse_websocket_frame(buffer);
        if (!payload.empty()) {
            std::cout << "Received message: " << payload << std::endl;
            messages_received++;
        }
    }

    // Optionally, send a close frame
    std::string close_message = "";
    std::vector<unsigned char> close_frame = create_websocket_frame(close_message);
    // To properly close, you need to set the opcode to 0x8 and FIN bit
    close_frame[0] = 0x88; // FIN + opcode 0x8
    std::string close_frame_str(close_frame.begin(), close_frame.end());
    ssl_send_all(ssl, close_frame_str);

    std::cout << "Received maximum number of messages. Closing connection." << std::endl;

    // Clean up
    SSL_shutdown(ssl);
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    close(sockfd);

    return EXIT_SUCCESS;
}
