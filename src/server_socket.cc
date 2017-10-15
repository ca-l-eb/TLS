#include <netdb.h>
#include <unistd.h>
#include <cstring>
#include <string>

#include "server_socket.h"

int cmd::server_socket::bind_server_socket(int port)
{
    int sock_fd = -1;
    struct addrinfo *addr;

    struct addrinfo hints {
    };
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;      // Accept IPv4 and IPv6
    hints.ai_socktype = SOCK_STREAM;  // Duplex connection (TCP probably)
    hints.ai_flags = AI_PASSIVE;      // Allow binding of socket for accepts

    const char *port_str = std::to_string(port).c_str();
    int ret = getaddrinfo(nullptr, port_str, &hints, &addr);
    if (ret != 0) {
        throw std::runtime_error("Could not create server socket: " +
                                 std::string(gai_strerror(ret)));
    }

    auto it = addr;
    for (; it != nullptr; it = it->ai_next) {
        sock_fd = ::socket(it->ai_family, it->ai_socktype, it->ai_protocol);
        if (sock_fd == -1)
            continue;

        ret = ::bind(sock_fd, addr->ai_addr, addr->ai_addrlen);
        if (ret == 0)
            break;  // Success

        ::close(sock_fd);  // Fail, try next
    }

    if (addr)
        freeaddrinfo(addr);

    if (it == nullptr)
        throw std::runtime_error("Could not bind socket to port " + std::to_string(port));
    return sock_fd;
}