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
    ~tls_socket() override;
    void connect(const std::string &host, int port) override;
    void close() override;
    ssize_t send(const void *buffer, size_t size, int flags) override;
    ssize_t send(const std::string &str, int flags) override;
    ssize_t recv(void *buffer, size_t size, int flags) override;
    ssize_t recv(std::vector<unsigned char> &buf, int flags) override;
    int get_fd() override;
    int get_port() override;
    std::string get_host() override;

private:
    cmd::tcp_socket sock;
    SSL *ssl;
};
}  // namespace cmd

#endif
