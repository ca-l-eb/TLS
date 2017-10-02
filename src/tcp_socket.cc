#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "tcp_socket.h"

cmd::tcp_socket::tcp_socket() : sock_fd{0} {}

cmd::tcp_socket::tcp_socket(int fd) : sock_fd{fd} {}

cmd::tcp_socket::~tcp_socket()
{
    close();
}

void cmd::tcp_socket::connect(const std::string &host, int port)
{
    connect_host(host.c_str(), port);
    this->host = host;
    this->port = port;
}

void cmd::tcp_socket::close()
{
    if (sock_fd >= 0)
        ::close(sock_fd);  // Ignore any errors
    sock_fd = -1;
}

int cmd::tcp_socket::send(const void *buffer, int size, int flags)
{
    return ::send(sock_fd, buffer, size, flags);
}

int cmd::tcp_socket::send(const std::string &str, int flags)
{
    return ::send(sock_fd, str.c_str(), str.size(), flags);
}

int cmd::tcp_socket::recv(void *buffer, int size, int flags)
{
    return ::recv(sock_fd, buffer, size, flags);
}

int cmd::tcp_socket::recv(std::vector<unsigned char> &buf, int flags)
{
    return ::recv(sock_fd, &buf[0], buf.capacity(), flags);
}

int cmd::tcp_socket::get_fd()
{
    return sock_fd;
}

void cmd::tcp_socket::connect_host(const std::string &host, int port)
{
    struct addrinfo *addr;
    struct addrinfo hints;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;      // Use IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;  // Request reliable, duplex connection (probably TCP)

    int ret = getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &addr);
    if (ret != 0) {
        throw std::runtime_error("Error getaddrinfo: " + std::string(gai_strerror(ret)));
    }

    // Loop through connections trying each
    auto i = addr;
    for (; i != NULL; i = i->ai_next) {
        // Create an unbound socket for the connection
        sock_fd = ::socket(i->ai_family, i->ai_socktype, i->ai_protocol);
        if (sock_fd == -1)
            continue;

        if (::connect(sock_fd, i->ai_addr, i->ai_addrlen) != -1)
            break;  // Success!

        ::close(sock_fd);  // close, try next
    }

    freeaddrinfo(addr);

    if (i == NULL)
        throw std::runtime_error(strerror(errno));  // Could not connect
}
