#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include "plain_socket.h"

static void connect_host(const std::string& host, int port, struct sockaddr_in *sin) {
    struct hostent *hp = gethostbyname(host.c_str());
    if (!hp)
        throw std::runtime_error("Could not find host " + host);
    memset(sin, 0, sizeof(struct sockaddr_in));
    sin->sin_family = AF_INET;
    memcpy((char*) &sin->sin_addr, hp->h_addr, hp->h_length);
    sin->sin_port = htons(port);
}

cmd::plain_socket::plain_socket() : sock_fd {0} {
    if ((sock_fd = ::socket(PF_INET, SOCK_STREAM, 0)) < 0)
        throw std::runtime_error(strerror(errno));
}

cmd::plain_socket::~plain_socket() {
    close();
}

void cmd::plain_socket::connect(const char *host, int port) {
    connect_host(host, port, &sin);
    if (::connect(sock_fd, (struct sockaddr*) &sin, sizeof(sin)) < 0)
        throw std::runtime_error(strerror(errno));
}

void cmd::plain_socket::connect(const std::string &host, int port) {
    connect(host.c_str(), port);
}

void cmd::plain_socket::close() {
    if (sock_fd >= 0 && (sock_fd = ::close(sock_fd)) == -1)
        throw std::runtime_error(strerror(errno));
}

void cmd::plain_socket::send(const char *buffer, int size, int flags) {
    ::send(sock_fd, buffer, size, flags);
}

void cmd::plain_socket::send(const uint8_t *buffer, int size, int flags) {
    ::send(sock_fd, buffer, size, flags);
}

void cmd::plain_socket::send(const std::string& str, int flags) {
    ::send(sock_fd, str.c_str(), str.size(), flags);
}

int cmd::plain_socket::recv(char *buffer, int size, int flags) {
    return ::recv(sock_fd, buffer, size, flags);
}

int cmd::plain_socket::recv(uint8_t *buffer, int size, int flags) {
    return ::recv(sock_fd, buffer, size, flags);
}

int cmd::plain_socket::recv(std::vector<char>& buf, int flags) {
    return ::recv(sock_fd, &buf[0], buf.capacity(), flags);
}
