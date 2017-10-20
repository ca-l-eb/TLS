#include <string.h>
#include <unistd.h>

#include <openssl/err.h>

#include "exceptions.h"
#include "ssl_manager.h"
#include "tls_socket.h"

static void throw_error_info(const std::string &msg)
{
    auto error = ERR_get_error();
    if (error != 0) {
        std::string error_string = std::string(ERR_error_string(error, nullptr));
        throw cmd::ssl_exception(msg + "::" + error_string);
    }
}

cmd::tls_socket::tls_socket(cmd::inet_family family) : family{family}
{
    ssl = SSL_new(cmd::ssl_manager::get_context());
    if (ssl == nullptr)
        throw_error_info("Could not create SSL connection object from SSL context");
}

cmd::tls_socket::tls_socket(int fd, SSL *ssl) : sock{fd}, ssl{ssl}
{
    // This constructor is intended to be called from tls_server::accept
    // fd should already be a connected socket, and ssl should be
    // an SSL instance already using fd, handshake complete
}

cmd::tls_socket::~tls_socket()
{
    close();
    SSL_free(ssl);
}

void cmd::tls_socket::connect(const std::string &host, int port)
{
    const char *const PREFERRED_CIPHERS = "HIGH:!aNULL:!kRSA:!PSK:!SRP:!MD5:!RC4";
    long ret = SSL_set_cipher_list(ssl, PREFERRED_CIPHERS);
    if (ret != 1)
        throw_error_info("Could not set cipher list");

    ret = SSL_set_tlsext_host_name(ssl, host.c_str());
    if (ret != 1)
        throw_error_info("Could not set TLS extension for hostname verification");

    sock.connect(host, port);
    int sock_fd = sock.get_fd();

    // Use underlying tcp_socket TCP connection for SSL calls
    ret = SSL_set_fd(ssl, sock_fd);
    if (ret != 1)
        throw_error_info("SSL could not use socket fd");

    // SSL handshake
    ret = SSL_connect(ssl);
    if (ret != 1)
        throw_error_info("Error completing TLS handshake");

    X509 *cert = SSL_get_peer_certificate(ssl);
    if (cert == nullptr)
        throw_error_info("Could not retrieved certificate from " + host);
    X509_free(cert);

    long verified = SSL_get_verify_result(ssl);
    if (verified != X509_V_OK)
        throw cmd::ssl_exception("Could not verify certificate for " + host);
}

void cmd::tls_socket::close()
{
    SSL_shutdown(ssl);
    SSL_clear(ssl);
    sock.close();
}

ssize_t cmd::tls_socket::send(const void *buffer, size_t size, int flags)
{
    int wrote = 0;
    while (size > 0) {
        int ret = SSL_write(ssl, buffer, static_cast<int>(size));
        if (ret > 0) {
            size -= ret;
            wrote += ret;
            continue;
        }
        int ssl_error = SSL_get_error(ssl, ret);
        switch (ssl_error) {
            case SSL_ERROR_ZERO_RETURN:
            // TLS connection closed
            case SSL_ERROR_SYSCALL:
                // TCP connection closed
                break;
            default:
                throw_error_info("Could not write to SSL connection");
        }
    }
    return wrote;
}

ssize_t cmd::tls_socket::send(const std::string &str, int flags)
{
    return send(str.c_str(), str.size(), flags);
}

ssize_t cmd::tls_socket::recv(void *buffer, size_t size, int flags)
{
    int ret = SSL_read(ssl, buffer, static_cast<int>(size));
    if (ret > 0)
        return ret;

    int ssl_error = SSL_get_error(ssl, ret);
    switch (ssl_error) {
        case SSL_ERROR_ZERO_RETURN:
        // TLS connection closed
        case SSL_ERROR_SYSCALL:
            // TCP connection closed
            break;
        default:
            throw_error_info("Could not write to SSL connection");
    }
    return ret;
}

ssize_t cmd::tls_socket::recv(std::vector<unsigned char> &buf, int flags)
{
    return recv(buf.data(), buf.size(), flags);
}

int cmd::tls_socket::get_fd()
{
    return sock.get_fd();
}

int cmd::tls_socket::get_port()
{
    return sock.get_port();
}

std::string cmd::tls_socket::get_host()
{
    return sock.get_host();
}
