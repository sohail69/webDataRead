#include "globalMacros.hpp"
#ifndef WEBSOCKET_HPP
#define WEBSOCKET_HPP 

// Include standard libraries
#include <iostream>
#include <string>
#include <cstring>
#include <iostream> 
#include <string> 
#include <sstream> 

// Include standard network libraries
#include <unistd.h> // For close()
#include <netdb.h>  // For getaddrinfo()
#include <arpa/inet.h>

// Include OpenSSL libraries
#include <openssl/ssl.h>
#include <openssl/err.h>

//Parses the data into a recognised struct
#include "JsonParser.hpp"

// Template function to connect and
// read from a WebSocket
//
template<typename dType>
class clientWebSocketIO{
  private:
    std::string path, hostname, port;
    SSL_CTX* ctx;
    SSL* ssl;
    int server_fd;
    std::string ConnStatus, sdType;
    const std::string kApiKey    = "PKZLV5L2XPYAFM5HVTLO";
    const std::string kApiSecret = "4zLDE7vbS1ABhNWlFNCJLgdeqsXyp45f3va0q8AO";

    void throwSSLError(std::string ErrMessage);
  public:
    clientWebSocketIO(const std::string path_
                    , const std::string host_
                    , const std::string port_);

    void readDatablock(std::string body, bool IsLastMessage);

    ~clientWebSocketIO();
};

// Buffer size for reading data
const int BUFFER_SIZE = 4096;


// Create an SSL_CTX object
SSL_CTX* create_context()
{
    const SSL_METHOD* method;
    SSL_CTX* ctx;
    method = TLS_client_method();
    ctx = SSL_CTX_new(method);
    if (!ctx){
        std::cerr << "Unable to create SSL context" << std::endl;
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    return ctx;
};

// Establish a TCP connection to the given hostname and port
int create_socket(const std::string& hostname, const std::string& port)
{
    struct addrinfo hints, *res, *p;
    int sockfd, connectfd;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;      // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;  // TCP stream sockets

    int status = getaddrinfo(hostname.c_str(), port.c_str(), &hints, &res);
    if (status != 0) std::cerr << "getaddrinfo: " << gai_strerror(status) << std::endl;
    if (status != 0) exit(EXIT_FAILURE);

    // Loop through all the results and connect to the first we can
    for(p = res; p != NULL; p = p->ai_next)
    {
      sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
      if (sockfd  == -1)   perror("socket"); //Socket Error
      if (sockfd  == -1)   continue;         //Look for next socket

      connectfd = connect(sockfd, p->ai_addr, p->ai_addrlen);
      if (connectfd == -1) perror("connect");//Connection Error
      if (connectfd == -1) close(sockfd);    //Close the socket

      if((connectfd != -1)and(sockfd  != -1)) break; // If we get here, we have successfully connected
    }

    if (p == NULL) std::cerr << "Failed to connect" << std::endl;
    if (p == NULL) exit(EXIT_FAILURE);

    freeaddrinfo(res); // Free the linked list
    return sockfd;
};


//Find an available socket and
//estabilish a connection
//
template<typename dType>
clientWebSocketIO<dType>::clientWebSocketIO(const std::string path_
                                          , const std::string  host_
                                          , const std::string port_)
                                          :path(path_), hostname(host_), port(port_){
    // Initialize OpenSSL
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();

    // Create SSL context
    ctx = create_context();

    // Create a TCP socket and connect to the server
    server_fd = create_socket(hostname, port);

    // Create an SSL object
    ssl = SSL_new(ctx);
    if (!ssl) throwSSLError( "Unable to create SSL structure");

    // **Set the SNI hostname**
    SSL_set_fd(ssl, server_fd);
    bool checkSNI = SSL_set_tlsext_host_name(ssl, hostname.c_str() );
    if(checkSNI != true) SSL_free(ssl);
    if(checkSNI != true) throwSSLError( "Error setting SNI hostname");

    // Perform SSL handshake
    int sslID = SSL_connect(ssl);
    if (sslID <= 0) SSL_free(ssl);
    if (sslID <= 0) throwSSLError( "SSL connection failed");
};


//Send a request and
//read in a data block
//
template<typename dType>
void clientWebSocketIO<dType>::readDatablock(std::string body, bool IsLastBlock){

    sdType = "application/json";// "*/*"
    if(IsLastBlock == true) ConnStatus = "close";
    if(IsLastBlock != true) ConnStatus = "keep-alive";


    // Formulate the HTTP GET request
    std::string request = "GET " + path + " HTTP/1.1\r\n"
                          "Host: " + hostname + "\r\n"
                          "User-Agent: MyApp (C++ HTTPS Client)\r\n"
                          "Accept: "+sdType+"\r\n"
                          "Connection: "+ConnStatus+"\r\n\r\n";

    std::cout << request << std::endl;
    std::cout << std::endl;
    std::cout << std::endl;

    // Send the request
    int bytes_sent = SSL_write(ssl, request.c_str(), request.length());
    if (bytes_sent <= 0) SSL_free(ssl);
    if (bytes_sent <= 0) throwSSLError("Failed to send request" );
 
    // Buffer to hold incoming data
    char buffer[BUFFER_SIZE];
    unsigned aggregate_bytes=0;
    std::string response = "";

    // Read the response
    while(true)
    {
      unsigned bytes_received = SSL_read(ssl, buffer, sizeof(buffer));
      aggregate_bytes = aggregate_bytes + bytes_received;
      if (bytes_received >  0) response.append(buffer, bytes_received);
      if(bytes_received <= 0) break;
      std::cout << response << std::endl;
    }
    Document document;
    document.Parse(response.c_str());

    // Separate headers and body
    std::string headers;
    size_t pos = response.find("\r\n\r\n");
    if (pos != std::string::npos) headers = response.substr(0, pos);
    if (pos != std::string::npos) body = response.substr(pos + 4);
    if (pos == std::string::npos) body = response; // No headers found, return entire response
};


// Close the connection
// and release the socket
//
template<typename dType>
clientWebSocketIO<dType>::~clientWebSocketIO(){
    // Close SSL connection
    std::string body;
    readDatablock(body, true);
    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(server_fd);
    SSL_CTX_free(ctx);
    EVP_cleanup();
};


// Throw an exception,
// close the connection
// and release the socket
//
template<typename dType>
void  clientWebSocketIO<dType>::throwSSLError(std::string ErrMessage){
  std::cerr <<  ErrMessage << std::endl;
  ERR_print_errors_fp(stderr);
  close(server_fd);
  SSL_CTX_free(ctx);
  EVP_cleanup();
  exit(EXIT_FAILURE);
}
#endif
