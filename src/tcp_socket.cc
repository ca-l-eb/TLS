#include <netdb.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "exceptions.h"
#include "inet_addr.h"
#include "tcp_socket.h"

cmd::tcp_socket::tcp_socket() : sock_fd{0}, port{-1}, host{} {}

cmd::tcp_socket::tcp_socket(int fd) : sock_fd{fd} {}

cmd::tcp_socket::~tcp_socket()
{
    close();
}

void cmd::tcp_socket::connect(const std::string &host, int port)
{
    connect_host(host, port);
    this->host = host;
    this->port = port;
}

void cmd::tcp_socket::close()
{
    if (sock_fd >= 0)
        ::close(sock_fd);  // Ignore any errors
    sock_fd = -1;
}

ssize_t cmd::tcp_socket::send(const void *buffer, size_t size, int flags)
{
    return ::send(sock_fd, buffer, size, flags);
}

ssize_t cmd::tcp_socket::send(const std::string &str, int flags)
{
    return ::send(sock_fd, str.c_str(), str.size(), flags);
}

ssize_t cmd::tcp_socket::recv(void *buffer, size_t size, int flags)
{
    return ::recv(sock_fd, buffer, size, flags);
}

ssize_t cmd::tcp_socket::recv(std::vector<unsigned char> &buf, int flags)
{
    return ::recv(sock_fd, &buf[0], buf.capacity(), flags);
}

int cmd::tcp_socket::get_fd()
{
    return sock_fd;
}

int cmd::tcp_socket::get_port()
{
    return port;
}

std::string cmd::tcp_socket::get_host()
{
    return host;
}

void cmd::tcp_socket::connect_host(const std::string &host, int port)
{
    cmd::inet_resolver resolver{host.c_str(), port, inet_proto ::tcp, inet_family::unspecified};

    // Loop through connections trying each
    for (auto &address : resolver.addresses) {
        // Create an unbound socket for the connection
        sock_fd =
            ::socket(address.addr.ai_family, address.addr.ai_socktype, address.addr.ai_protocol);
        if (sock_fd == -1)
            continue;

        if (::connect(sock_fd, address.addr.ai_addr, address.addr.ai_addrlen) != -1)
            break;  // Success!

        ::close(sock_fd);  // close, try next
    }
    throw cmd::socket_exception("Could not connected to host: " + host);  // Could not connect
}
