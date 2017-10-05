#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <exception>
#include <memory>

#include "tcp_server.h"
#include "tcp_socket.h"

cmd::tcp_server::tcp_server() : sock_fd{-1}, port{-1} {}

cmd::tcp_server::~tcp_server()
{
    close();
}

cmd::socket::ptr cmd::tcp_server::accept()
{
    // We don't care about the details of who connected (nullptr, nullptr)
    int client_fd = ::accept(sock_fd, nullptr, nullptr);
    if (client_fd == -1)
        throw std::runtime_error("Accept failed: " + std::string(std::strerror(errno)));
    return std::make_shared<cmd::tcp_socket>(client_fd);
}

void cmd::tcp_server::bind(int port)
{
    if (sock_fd >= 0)
        return;

    sock_fd = bind_server_socket(port);
    this->port = port;
}

void cmd::tcp_server::close()
{
    if (sock_fd >= 0)
        ::close(sock_fd);
    sock_fd = -1;
}

void cmd::tcp_server::listen(int waiting)
{
    int ret = ::listen(sock_fd, waiting);
    if (ret != 0)
        throw std::runtime_error(std::strerror(errno));
}

int cmd::tcp_server::get_port()
{
    return port;
}
