#include <poll.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>

#include "exceptions.h"
#include "udp_socket.h"

cmd::udp_socket::udp_socket(inet_family family) : sock_fd{-1}
{
    switch (family) {
        case inet_family::ipv4:
            sock_fd = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if (sock_fd >= 0)
                break;
            throw cmd::socket_exception{"Could not make ipv4 UDP socket"};
        case inet_family::ipv6:
            sock_fd = ::socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
            if (sock_fd >= 0)
                break;
            throw cmd::socket_exception{"Could not make ipv6 UDP socket"};
        default:
            throw cmd::socket_exception{"Unsupported internet family for UDP: " +
                                        std::to_string((int) family)};
    }
}

cmd::udp_socket::~udp_socket()
{
    close();
}

void cmd::udp_socket::bind(inet_addr &address)
{
    int ret = ::bind(sock_fd, address.addr.ai_addr, address.addr.ai_addrlen);
    if (ret != -1) {
        addr = address;
        return;
    }
    throw cmd::socket_exception{"Could not bind addresses to UDP socket: " +
                                std::string(std::strerror(ret))};
}

void cmd::udp_socket::close()
{
    if (sock_fd >= 0) {
        ::close(sock_fd);
        sock_fd = -1;
    }
}

ssize_t cmd::udp_socket::send(const cmd::inet_addr &addr, const void *buffer, size_t size,
                              int flags)
{
    return sendto(sock_fd, buffer, size, flags, addr.addr.ai_addr, addr.addr.ai_addrlen);
}

ssize_t cmd::udp_socket::send(const cmd::inet_addr &addr, const std::string &str, int flags)
{
    return send(addr, str.c_str(), str.size(), flags);
}

ssize_t cmd::udp_socket::send(const void *buffer, size_t size, int flags)
{
    return send(addr, buffer, size, flags);
}

ssize_t cmd::udp_socket::send(const std::string &str, int flags)
{
    return send(addr, str.c_str(), str.size(), flags);
}

ssize_t cmd::udp_socket::recv(void *buffer, size_t size, int flags)
{
    return ::recv(sock_fd, buffer, size, flags);
}

ssize_t cmd::udp_socket::recv(void *buffer, size_t size, int flags, int timeout_ms)
{
    pollfd fds[1];
    fds[0].fd = sock_fd;
    fds[0].events = POLLIN;
    auto ret = poll(fds, 1, timeout_ms);

    if (ret == 0 || fds[0].revents & POLLERR) {
        // We didn't get POLLIN in time
        return 0;
    } else if (ret == -1) {
        throw cmd::socket_exception{"Poll error: " + std::string{std::strerror(errno)}};
    } else {
        return recv(buffer, size, flags);
    }
}

ssize_t cmd::udp_socket::recv(cmd::inet_addr &from, void *buffer, size_t size, int flags)
{
    return ::recvfrom(sock_fd, buffer, size, flags, from.addr.ai_addr, &from.addr.ai_addrlen);
}

ssize_t cmd::udp_socket::recv(cmd::inet_addr &from, void *buffer, size_t size, int flags,
                              int timeout_ms)
{
    pollfd fds[1];
    fds[0].fd = sock_fd;
    fds[0].events = POLLIN;
    auto ret = poll(fds, 1, timeout_ms);

    // We didn't get POLLIN in time
    if (ret == 0 || fds[0].revents & POLLERR) {
        return 0;
    } else if (ret == -1) {
        throw cmd::socket_exception{"Poll: " + std::string{std::strerror(errno)}};
    } else {
        return recv(from, buffer, size, flags);
    }
}

const cmd::inet_addr cmd::udp_socket::get_address() const
{
    return addr;
}

int cmd::udp_socket::get_fd()
{
    return sock_fd;
}
