#include <netdb.h>
#include <unistd.h>
#include <cstring>
#include <string>

#include "exceptions.h"
#include "inet_addr.h"
#include "server_socket.h"

int cmd::server_socket::bind_server_socket(int port, cmd::inet_family family)
{
    int sock_fd = -1;
    cmd::inet_resolver resolver{nullptr, port, inet_proto::tcp, family};

    for (auto &address : resolver.addresses) {
        sock_fd =
            ::socket(address.addr.ai_family, address.addr.ai_socktype, address.addr.ai_protocol);
        if (sock_fd == -1)
            continue;

        if (::bind(sock_fd, address.addr.ai_addr, address.addr.ai_addrlen) == 0)
            return sock_fd;  // Success

        ::close(sock_fd);  // Fail, try next
    }
    throw cmd::socket_exception("Could not bind socket to port " + std::to_string(port));
}