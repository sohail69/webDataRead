#pragma once
// Include my libraries
#include "globalMacros.hpp"
#include "randomKeyGen.hpp"

// Include standard libraries
#include <iostream>
#include <sstream> 
#include <string>
#include <cstring>

// Include standard network libraries
#include <unistd.h> // For close()
#include <netdb.h>  // For getaddrinfo()
#include <arpa/inet.h>

// Include OpenSSL libraries
#include <openssl/ssl.h>
#include <openssl/err.h>



/*****************************************\
!
!  Template function to connect and
!      read from a WebSocket
!
\*****************************************/
template<typename dType>
class clientWebSocketIO{
  private:
    // Buffer size for reading data
    const int BUFFER_SIZE = 4096;

    //OpenSSL connection data 
    struct addrinfo hints, *server_info, *p;
    int server_fd, connectfd;
    SSL_CTX* ctx=NULL;
    SSL* ssl=NULL;

    //Connection Data
    std::string path, hostname, port;
    std::string ConnStatus, sdType;
    const std::string kApiKey    = "PKZLV5L2XPYAFM5HVTLO";
    const std::string kApiSecret = "4zLDE7vbS1ABhNWlFNCJLgdeqsXyp45f3va0q8AO";

    void closeOutSSLConnection(std::string ErrMessage, bool IsErr);
    void create_socket(const std::string& hostname, const std::string& port);
    bool ssl_receive(const SSL* ssl_, std::string & received);
    bool ssl_send_all(SSL* ssl, const std::string& data);
    std::string construct_handshake_request(const std::string& websocket_key);
  public:
    clientWebSocketIO(const std::string path_
                    , const std::string host_
                    , const std::string port_);

    void readDatablock(std::string & response, const std::string & request);

    ~clientWebSocketIO();
};



/*****************************************\
!
!  Establish a TCP connection to the 
!       given hostname and port
!
\*****************************************/
template<typename dType>
void clientWebSocketIO<dType>::create_socket(const std::string& hostname, const std::string& port)
{
    memset(&hints, 0, sizeof hints);
    hints.ai_family   = AF_UNSPEC;    // IPv4 or IPv6  // AF_INET;       // IPv4
    hints.ai_socktype = SOCK_STREAM;  // TCP stream sockets

    int status = getaddrinfo(hostname.c_str(), port.c_str(), &hints, &server_info);
    if (status != 0) std::cerr << "getaddrinfo: " << gai_strerror(status) << std::endl;
    if (status != 0) exit(EXIT_FAILURE);

    // Loop through all the results and connect to the first we can
    for(p = server_info; p != NULL; p = p->ai_next)
    {
      server_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
      if (server_fd  < 0)   perror("socket"); //Socket Error
      if (server_fd  < 0)   continue;         //Look for next socket

      connectfd = connect(server_fd, p->ai_addr, p->ai_addrlen);
      if (connectfd == -1) perror("connect");//Connection Error
      if (connectfd == -1) close(server_fd); //Close the socket

      if((connectfd != -1)and(server_fd  != -1)) break; // If we get here, we have successfully connected
    }

    freeaddrinfo(server_info); // Free the linked list
    if (p == NULL) std::cerr << "Failed to connect" << std::endl;
    if (p == NULL) exit(EXIT_FAILURE);
};


/*****************************************\
!
!  Constructs the WebSocket handshake
!          HTTP request.
!
\*****************************************/
 template<typename dType>
std::string clientWebSocketIO<dType>::construct_handshake_request(const std::string& websocket_key){
  std::stringstream ss;
  ss << "GET "   << path     << " HTTP/1.1\r\n";
  ss << "Host: " << hostname << "\r\n";
  ss << "Upgrade: websocket\r\n";
  ss << "Connection: Upgrade\r\n";
  ss << "Sec-WebSocket-Key: " << websocket_key << "\r\n";
  ss << "Sec-WebSocket-Version: 13\r\n";
  ss << "\r\n";
  return ss.str();
}


/*****************************************\
!
!  Find an available socket and
!    estabilish a connection
!
\*****************************************/
template<typename dType>
clientWebSocketIO<dType>::clientWebSocketIO(const std::string path_
                                          , const std::string host_
                                          , const std::string port_)
                                          :path(path_), hostname(host_), port(port_){
  // Initialize OpenSSL
  SSL_library_init();
  SSL_load_error_strings();
  OpenSSL_add_ssl_algorithms();

  // Create a TCP socket and connect to the server
  create_socket(hostname, port);

  // Create SSL context
  ctx = SSL_CTX_new(TLS_client_method());
  if(!ctx) closeOutSSLConnection("Unable to create SSL context", true);

  // Create an SSL object
  ssl = SSL_new(ctx);
  if (!ssl) closeOutSSLConnection( "SSL_new failed", true);

  // **Set the SNI hostname**
  SSL_set_fd(ssl, server_fd);
  bool checkSNI = SSL_set_tlsext_host_name(ssl, hostname.c_str() );
  if(checkSNI != true) closeOutSSLConnection( "Error setting SNI hostname", true);

  // Connect via SSL
  int sslID = SSL_connect(ssl);
  if (sslID <= 0) closeOutSSLConnection( "SSL connection failed", true);

  // Generate Sec-WebSocket-Key
  // and request for handshake
  std::string websocket_key     = generate_websocket_key();
  std::string handshake_request = construct_handshake_request(websocket_key);


  // Send handshake request
  // and get response
  bool checkHandshake = ssl_send_all(ssl, handshake_request);
  if (!checkHandshake) closeOutSSLConnection("Failed to send handshake request", true);
//  std::string handshake_response = ssl_receive(ssl);
//  std::cout << "Handshake Response:\n" << handshake_response << std::endl;
};


/*****************************************\
!
!  Send a request and read in
!      a data block
!
\*****************************************/
template<typename dType>
void clientWebSocketIO<dType>::readDatablock(std::string & response, const std::string & request){
  // Send the request
  int bytes_sent = SSL_write(ssl, request.c_str(), request.length());
  if (bytes_sent <= 0) SSL_free(ssl);
  if (bytes_sent <= 0) closeOutSSLConnection("Failed to send request",true);
 
  // Buffer to hold incoming data
  char buffer[BUFFER_SIZE];
  unsigned aggregate_bytes=0, bytes_received=0;
  std::string tot_response = "";
  response = "";

  // Read the response
  while( (bytes_received = SSL_read(ssl, buffer, sizeof(buffer))) > 0)
  {
    aggregate_bytes = aggregate_bytes + bytes_received;
    //Need a better measurement of when to stop
    if (bytes_received >  0) tot_response.append(buffer, bytes_received);
    std::cout << aggregate_bytes << std::endl;
    if(tot_response.find("\r\n\r\n") != std::string::npos) break;
  }

  // Separate headers and body
  std::string headers, footers;
  size_t pos = tot_response.find("\r\n\r\n");
  if (pos != std::string::npos) headers  = tot_response.substr(0, pos);
  if (pos != std::string::npos) response = tot_response.substr(pos + 4);
  if (pos == std::string::npos) response = tot_response; // No headers found, return entire response
};


/*****************************************\
!
!  Helper function to receive data 
!       over SSL connection.
!
\*****************************************/
template<typename dType>
bool clientWebSocketIO<dType>::ssl_receive(const SSL* ssl_, std::string & received){
  char buffer[BUFFER_SIZE];
  int bytes;
  while ((bytes = SSL_read(ssl_, buffer, BUFFER_SIZE)) > 0) {
    received.append(buffer, bytes);
    // WebSocket handshake response ends with double CRLF
    if(received.find("\r\n\r\n") != std::string::npos) break;
  }
  return false;
};


/*****************************************\
!
!  Helper function to send data
!      over SSL connection.
!
\*****************************************/
template<typename dType>
bool clientWebSocketIO<dType>::ssl_send_all(SSL* ssl, const std::string& data) {
    int total_sent = 0;
    int data_len = data.length();
    const char* buf = data.c_str();

    while (total_sent < data_len) {
        int sent = SSL_write(ssl, buf + total_sent, data_len - total_sent);
        if (sent <= 0) std::cerr << "SSL_write failed." << std::endl;
        if (sent <= 0) return false;
        total_sent += sent;
    }
    return true;
}


/*****************************************\
!
!  Close the connection and release
!          the socket
!
\*****************************************/
template<typename dType>
clientWebSocketIO<dType>::~clientWebSocketIO(){ closeOutSSLConnection(" ", false); };


/*****************************************\
!
!  Throws an exception if there is
!    an error otherwise closes
!      connection gracefully
!
\*****************************************/
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
