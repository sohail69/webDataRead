#pragma once
// Include standard libraries
#include <sstream> 
#include <string>
#include <cstring>
#include <random>
#include <algorithm>

/**
 * Helper function to perform Base64 encoding.
 * This is required for creating the Sec-WebSocket-Key header.
 */
std::string base64_encode(const unsigned char* input, int length) {
    static const char encoding_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string encoded;
    int padding = 0;

    if (length % 3 != 0) padding = 3 - (length % 3);
    int encoded_length = 4 * ((length + padding) / 3);
    encoded.reserve(encoded_length);

    int i = 0;
    for (; i + 2 < length; i += 3) {
        encoded += encoding_table[(input[i] >> 2) & 0x3F];
        encoded += encoding_table[((input[i] & 0x3) << 4) | ((input[i + 1] >> 4) & 0xF)];
        encoded += encoding_table[((input[i + 1] & 0xF) << 2) | ((input[i + 2] >> 6) & 0x3)];
        encoded += encoding_table[input[i + 2] & 0x3F];
    }

    if (padding > 0) {
        unsigned char byte1 = input[i];
        encoded += encoding_table[(byte1 >> 2) & 0x3F];
        if (padding == 1) {
            encoded += encoding_table[((byte1 & 0x3) << 4)];
            encoded += "==";
        } else if (padding == 2) {
            unsigned char byte2 = input[i + 1];
            encoded += encoding_table[((byte1 & 0x3) << 4) | ((byte2 >> 4) & 0xF)];
            encoded += encoding_table[((byte2 & 0xF) << 2)];
            encoded += "=";
        }
    }

    return encoded;
}

/**
 * Generates a random string for the Sec-WebSocket-Key header.
 */
std::string generate_websocket_key() {
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distribution(0, 255);
    unsigned char key[16];
    for (int i = 0; i < 16; ++i) {
        key[i] = distribution(generator);
    }
    return base64_encode(key, 16);
}

/**
 * Computes the SHA1 hash and returns it as a string.
 */
/*
std::string sha1_hash(const std::string& input) {
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(input.c_str()), input.size(), hash);
    return std::string(reinterpret_cast<char*>(hash), SHA_DIGEST_LENGTH);
}*/

