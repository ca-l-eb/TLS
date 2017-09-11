#include <string>
#include <stdexcept>
#include <iostream>
#include <unistd.h>
#include <errno.h>

#include <openssl/err.h>

#include "tls_socket.h"
#include "ssl_manager.h"

static void throw_error_info(const std::string &msg) {
    int error = ERR_get_error();
    if (error != 0) {
        std::string error_string = std::string(ERR_error_string(error, NULL));
        throw std::runtime_error(msg + "::" + error_string);
    }
}

static void check_ssl_error(SSL *ssl, int ret, const std::string &msg) {
    int ssl_error = SSL_get_error(ssl, ret);
    switch (ssl_error) {
        case SSL_ERROR_NONE:
            // No error
            break;
        case SSL_ERROR_ZERO_RETURN:
            // Connection closed cleanly
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
            throw_error_info(msg);
            break;
        case SSL_ERROR_SSL:
            std::cerr << "SSL_ERROR_SSL\n";
            throw_error_info(msg);
            break;
    }
}

cmd::tls_socket::tls_socket(SSL_CTX *context) {
    connection = BIO_new_ssl_connect(context);
    if (connection == NULL) {
        throw_error_info("Could not create SSL connection object from SSL context");
    }
}

cmd::tls_socket::~tls_socket() {
    close();
    if (connection)
        BIO_free_all(connection);
}

void cmd::tls_socket::connect(const char *host, int port) {
    std::string full_host = std::string(host);
    full_host += ":" + std::to_string(port);

    long ret = BIO_set_conn_hostname(connection, full_host.c_str());
    if (ret != 1)
        check_ssl_error(ssl, ret, "Could not find host " + full_host);

    // Get ssl object from BIO object
    BIO_get_ssl(connection, &ssl);
    if (ssl == NULL)
        throw_error_info("Could not get SSL object from BIO");

    const char* const PREFERRED_CIPHERS = "HIGH:!aNULL:!kRSA:!PSK:!SRP:!MD5:!RC4";
    ret = SSL_set_cipher_list(ssl, PREFERRED_CIPHERS);
    if (ret != 1)
        throw_error_info("Could not set cipher list");

    ret = SSL_set_tlsext_host_name(ssl, host);
    if (ret != 1)
        throw_error_info("Could not set TLS extension for hostname verification");

    // TCP connect
    ret = BIO_do_connect(connection);
    if (ret != 1)
        throw_error_info("Could not connect to " + full_host);

    // SSL handshake
    ret = BIO_do_handshake(connection);
    if (ret != 1)
        throw_error_info("Error completing TLS handshake");

    X509 *cert = SSL_get_peer_certificate(ssl);
    if (cert == NULL)
        throw_error_info("Could not retrieved certificate from " + full_host);
    X509_free(cert);

    long verified = SSL_get_verify_result(ssl);
    if (verified != X509_V_OK)
        throw std::runtime_error("Could not verify certificate for " + full_host);
}

void cmd::tls_socket::connect(const std::string &host, int port) {
    connect(host.c_str(), port);
}

void cmd::tls_socket::close() {
    SSL_shutdown(ssl);
}

void cmd::tls_socket::send(const char *buffer, int size, int flags) {
    int ret = 0;
    while (size > 0) {
        ret = BIO_write(connection, buffer, size);
        if (ret > 0) {
            size -= ret;
            continue;
        }
        check_ssl_error(ssl, ret, "Could not write to SSL connection");
    }
}

void cmd::tls_socket::send(const uint8_t *buffer, int size, int flags) {
    send(buffer, size, flags);
}

void cmd::tls_socket::send(const std::string& str, int flags) {
    send(str.c_str(), str.size(), flags);
}

int cmd::tls_socket::recv(char *buffer, int size, int flags) {
    int ret = BIO_read(connection, buffer, size);
    if (ret > 0)
        return ret;

    check_ssl_error(ssl, ret, "Could not read from SSL connection");
    return 0;
}

int cmd::tls_socket::recv(uint8_t *buffer, int size, int flags) {
    return recv(reinterpret_cast<char*>(buffer), size, flags);
}

int cmd::tls_socket::recv(std::vector<char>& buf, int flags) {
    return recv(static_cast<char*>(&buf[0]), buf.size(), flags);
}
