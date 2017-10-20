#ifndef CMD_TLS_SERVER_H
#define CMD_TLS_SERVER_H

#include <string>

#include "inet_addr.h"
#include "server_socket.h"

namespace cmd
{
class tls_server : public cmd::server_socket
{
public:
    tls_server(const std::string &cert, const std::string &privkey,
               cmd::inet_family family = cmd::inet_family::unspecified);
    tls_server(const tls_server &) = delete;
    tls_server &operator=(const tls_server &) = delete;
    ~tls_server() override;
    cmd::socket::ptr accept() override;
    void bind(int port) override;
    void close() override;
    void listen(int waiting) override;
    int get_port() override;

private:
    SSL_CTX *context;
    int sock_fd;
    int port;
    cmd::inet_family family;
};
}  // namespace cmd

#endif