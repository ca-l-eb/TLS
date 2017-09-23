#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <exception>
#include <iostream>
#include <memory>

#include "tcp_server.h"
#include "tcp_socket.h"

cmd::tcp_server::tcp_server() : sock_fd{-1} {}

cmd::tcp_server::~tcp_server()
{
    close();
}

cmd::socket::ptr cmd::tcp_server::accept()
{
    // We don't care about the details of who connected (NULL, NULL)
    int client_fd = ::accept(sock_fd, NULL, NULL);
    if (client_fd == -1)
        throw std::runtime_error("Accept failed: " + std::string(std::strerror(errno)));
    return std::make_shared<cmd::tcp_socket>(client_fd);
}

void cmd::tcp_server::bind(int port)
{
    static bool init = false;
    if (init)
        return;

    sock_fd = bind_server_socket(port);

    init = true;
}

void cmd::tcp_server::close()
{
    if (sock_fd > 0)
        ::close(sock_fd);
}

void cmd::tcp_server::listen(int waiting)
{
    int ret = ::listen(sock_fd, waiting);
    if (ret != 0)
        throw std::runtime_error(std::strerror(errno));
}

int cmd::bind_server_socket(int port)
{
    int sock_fd = -1;
    struct addrinfo *addr;

    struct addrinfo hints;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;      // Accept IPv4 and IPv6
    hints.ai_socktype = SOCK_STREAM;  // Duplx connection (TCP probably)
    hints.ai_flags = AI_PASSIVE;      // Allow binding of socket

    const char *port_str = std::to_string(port).c_str();
    int ret = getaddrinfo(NULL, port_str, &hints, &addr);
    if (ret != 0) {
        throw std::runtime_error("Could not create server socket: " +
                                 std::string(gai_strerror(ret)));
    }

    auto it = addr;
    for (; it != NULL; it = it->ai_next) {
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

    if (it == NULL)
        throw std::runtime_error("Could not bind socket to port " + std::to_string(port));
    return sock_fd;
}
