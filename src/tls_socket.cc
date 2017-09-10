#include <string>
#include <stdexcept>
#include <iostream>
#include <unistd.h>
#include <errno.h>

#include <openssl/err.h>

#include "tls_socket.h"
#include "ssl_manager.h"

static void print_error_info() {
    int error = ERR_get_error();
    if (error != 0)
        std::cerr << ERR_error_string(error, NULL) << "\n";
}

static void check_ssl_error(SSL *ssl, int ret) {
    int ssl_error = SSL_get_error(ssl, ret);
    switch (ssl_error) {
        case SSL_ERROR_NONE:
            std::cerr << "SSL_ERROR_NONE\n";
            break;
        case SSL_ERROR_ZERO_RETURN:
            std::cerr << "SSL_ERROR_ZERO_RETURN\n";
            break;
        case SSL_ERROR_WANT_READ:
            std::cerr << "SSL_ERROR_WANT_READ\n";
            break;
        case SSL_ERROR_WANT_WRITE:
            std::cerr << "SSL_ERROR_WANT_WRITE\n";
            break;
        case SSL_ERROR_WANT_CONNECT:
            std::cerr << "SSL_ERROR_WANT_CONNECT\n";
            break;
        case SSL_ERROR_WANT_ACCEPT:
            std::cerr << "SSL_ERROR_WANT_ACCEPT\n";
            break;
        case SSL_ERROR_WANT_X509_LOOKUP:
            std::cerr << "SSL_ERROR_WANT_X509_LOOKUP\n";
            break;
        case SSL_ERROR_SYSCALL:
            std::cerr << "SSL_ERROR_SYSCALL\n";
            break;
        case SSL_ERROR_SSL:
            std::cerr << "SSL_ERROR_SSL\n";
            break;
    }
    print_error_info();
}

cmd::tls_socket::tls_socket(SSL_CTX *context) {
    ssl = SSL_new(context);
}

cmd::tls_socket::~tls_socket() {
    close();
    SSL_free(ssl);
}

void cmd::tls_socket::connect(const char *host, int port) {
    sock.connect(host, port);
    int sock_fd = sock.get_fd();
    int error = 0;

    // Attach the SSL object to the existing TCP connection
    int ret = SSL_set_fd(ssl, sock_fd);
    print_error_info();

    ret = SSL_connect(ssl);
    if (ret != 1) {
        check_ssl_error(ssl, ret);
        throw std::runtime_error("Could not connect with SSL to host " + std::string(host));
    }

    X509 *cert = SSL_get_peer_certificate(ssl);
    if (cert == NULL) {
        print_error_info();
        throw std::runtime_error("Could not retrieve certificate from host " + std::string(host));
    }
    X509_free(cert);

    long verified = SSL_get_verify_result(ssl);
    if (verified != X509_V_OK) {
        throw std::runtime_error("Could not verify host " + std::string(host));
    }
}

void cmd::tls_socket::connect(const std::string &host, int port) {
    connect(host.c_str(), port);
}

void cmd::tls_socket::close() {
    SSL_shutdown(ssl);
    sock.close();
}

void cmd::tls_socket::send(const char *buffer, int size, int flags) {
    int ret = 0;
    while (size > 0) {
        ret = SSL_write(ssl, buffer + ret, size - ret);
        if (ret > 0)
            continue;

        check_ssl_error(ssl, ret);
        throw std::runtime_error("Could not write to SSL connection");
    }
}

void cmd::tls_socket::send(const uint8_t *buffer, int size, int flags) {
    send(buffer, size, flags);
}

void cmd::tls_socket::send(const std::string& str, int flags) {
    send(str.c_str(), str.size(), flags);
}

int cmd::tls_socket::recv(char *buffer, int size, int flags) {
    int ret = SSL_read(ssl, buffer, size);
    if (ret > 0)
        return ret;
    int error_code = SSL_get_error(ssl, ret);
    char *e = ERR_error_string(error_code, NULL);
    std::cout << e << "\n";
    throw std::runtime_error("Error code: " + std::to_string(error_code));
}

int cmd::tls_socket::recv(uint8_t *buffer, int size, int flags) {
    return recv(reinterpret_cast<char*>(buffer), size, flags);
}

int cmd::tls_socket::recv(std::vector<char>& buf, int flags) {
    return recv(static_cast<char*>(&buf[0]), buf.size(), flags);
}
