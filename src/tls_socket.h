#ifndef CMD_TLS_SOCKET
#define CMD_TLS_SOCKET

#include <openssl/ssl.h>
#include <string>

#include "socket.h"
#include "ssl_manager.h"
#include "tcp_socket.h"

namespace cmd
{
class tls_socket : public cmd::socket
{
public:
    tls_socket();
    tls_socket(int fd, SSL *ssl);
    ~tls_socket();
    void connect(const std::string &host, int port);
    void close();
    int send(const void *buffer, int size, int flags = 0);
    int send(const std::string &str, int flags = 0);
    int recv(void *buffer, int size, int flags = 0);
    int recv(std::vector<unsigned char> &buf, int flags = 0);
    int get_fd();

private:
    cmd::tcp_socket sock;
    SSL *ssl;
};
};

#endif
