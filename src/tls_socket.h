#ifndef CMD_TLS_SOCKET
#define CMD_TLS_SOCKET

#include <openssl/ssl.h>
#include <string>

#include "inet_addr.h"
#include "socket.h"
#include "ssl_manager.h"
#include "tcp_socket.h"

namespace cmd
{
class tls_socket : public cmd::socket
{
public:
    explicit tls_socket(cmd::inet_family family = cmd::inet_family::unspecified);
    tls_socket(const tls_socket &) = delete;
    tls_socket &operator=(const tls_socket &) = delete;
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
    cmd::inet_family family;
};
}  // namespace cmd

#endif
