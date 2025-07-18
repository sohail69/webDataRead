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
    SSL_CTX* ctx=NULL;
    SSL* ssl=NULL;
    int server_fd;
    std::string ConnStatus, sdType;
    const std::string kApiKey    = "PKZLV5L2XPYAFM5HVTLO";
    const std::string kApiSecret = "4zLDE7vbS1ABhNWlFNCJLgdeqsXyp45f3va0q8AO";

    void closeOutSSLConnection(std::string ErrMessage, bool IsErr);
  public:
    clientWebSocketIO(const std::string path_
                    , const std::string host_
                    , const std::string port_);

    void readDatablock(std::string & response, const std::string & request);

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
    if (!ssl) closeOutSSLConnection( "Unable to create SSL structure", true);

    // **Set the SNI hostname**
    SSL_set_fd(ssl, server_fd);
    bool checkSNI = SSL_set_tlsext_host_name(ssl, hostname.c_str() );
    if(checkSNI != true) closeOutSSLConnection( "Error setting SNI hostname", true);

    // Perform SSL handshake
    int sslID = SSL_connect(ssl);
    if (sslID <= 0) closeOutSSLConnection( "SSL connection failed", true);
};


//Send a request and
//read in a data block
//
template<typename dType>
void clientWebSocketIO<dType>::readDatablock(std::string & response, const std::string & request){
    // Send the request
    int bytes_sent = SSL_write(ssl, request.c_str(), request.length());
    if (bytes_sent <= 0) SSL_free(ssl);
    if (bytes_sent <= 0) closeOutSSLConnection("Failed to send request",true);
 
    // Buffer to hold incoming data
    char buffer[BUFFER_SIZE];
    unsigned aggregate_bytes=0;
    std::string tot_response = "";
    response = "";

    // Read the response
    while(true)
    {
      unsigned bytes_received = SSL_read(ssl, buffer, sizeof(buffer));
      aggregate_bytes = aggregate_bytes + bytes_received;
      //Need a better measurement of when to stop
      if (bytes_received >  0) tot_response.append(buffer, bytes_received);
      if(bytes_received <= 0) break;
    }

    // Separate headers and body
    std::string headers;
    size_t pos = tot_response.find("\r\n\r\n");
    if (pos != std::string::npos) headers = tot_response.substr(0, pos);
    if (pos != std::string::npos) response = tot_response.substr(pos + 4);
    if (pos == std::string::npos) response = tot_response; // No headers found, return entire response
};

// Close the connection
// and release the socket
//
template<typename dType>
clientWebSocketIO<dType>::~clientWebSocketIO(){ closeOutSSLConnection(" ", false); };

// Throws an exception if there is
// an error otherwise closes
// connection gracefully
//
template<typename dType>
void  clientWebSocketIO<dType>::closeOutSSLConnection(std::string ErrMessage, bool IsErr){
  if(IsErr)  std::cerr <<  ErrMessage << std::endl;
  if(IsErr) ERR_print_errors_fp(stderr);
  if(ssl != NULL) SSL_shutdown(ssl);
  if(ssl != NULL) SSL_free(ssl);
  close(server_fd);
  if(ctx != NULL) SSL_CTX_free(ctx);
  EVP_cleanup();
  if(IsErr) exit(EXIT_FAILURE);
};
#endif
