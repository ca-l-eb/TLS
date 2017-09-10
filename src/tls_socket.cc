#include <tls.h>
#include <string>
#include <stdexcept>
#include <iostream>

#include "tls_socket.h"

cmd::tls_socket::tls_socket(struct tls_config *tls_conf) {
    client = tls_client();
    tls_configure(client, tls_conf);
}

cmd::tls_socket::~tls_socket() {
    close();
    tls_free(client);
}

void cmd::tls_socket::connect(const char *host, int port) {
    const char *port_str = std::to_string(port).c_str();
    if (tls_connect(client, host, port_str) == -1)
        throw std::runtime_error("Could not connect to host " + std::string(host));
}

void cmd::tls_socket::connect(const std::string &host, int port) {
    connect(host.c_str(), port);
}

void cmd::tls_socket::close() {
    tls_close(client);
}

void cmd::tls_socket::send(const char *buffer, int size, int flags) {
    while (size > 0) { 
        ssize_t ret = tls_write(client, buffer, size); 
        if (ret == TLS_WANT_POLLIN || ret == TLS_WANT_POLLOUT) 
            continue; 
        if (ret < 0) 
            throw std::runtime_error("tls_write: " + std::string(tls_error(client))  +"\n"); 
        buffer += ret; 
        size -= ret; 
    }
}

void cmd::tls_socket::send(const uint8_t *buffer, int size, int flags) {
    send(buffer, size, flags);
}

void cmd::tls_socket::send(const std::string& str, int flags) {
    send(str.c_str(), str.size(), flags);
}

int cmd::tls_socket::recv(char *buffer, int size, int flags) {
    return tls_read(client, buffer, size);
}

int cmd::tls_socket::recv(uint8_t *buffer, int size, int flags) {
    return tls_read(client, buffer, size);
}

int cmd::tls_socket::recv(std::vector<char>& buf, int flags) {
    return tls_read(client, &buf[0], buf.capacity());
}
