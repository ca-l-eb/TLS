#ifndef CMD_TLS_SERVER_H
#define CMD_TLS_SERVER_H

#include "server_socket.h"
#include "tcp_server.h"

namespace cmd
{
class tls_server : public cmd::server_socket
{
public:
    tls_server(const char *cert, const char *privkey);
    ~tls_server();
    cmd::socket::ptr accept();
    void bind(int port);
    void close();
    void listen(int waiting);

private:
    SSL_CTX *context;
    int sock_fd;
    ;
};
}

#endif
