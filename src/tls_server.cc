#include <unistd.h>
#include <cerrno>
#include <cstdio>
#include <cstring>

#include <openssl/err.h>
#include <openssl/ssl.h>
#include <memory>

#include "exceptions.h"
#include "ssl_manager.h"
#include "tls_server.h"
#include "tls_socket.h"

cmd::tls_server::tls_server(const std::string &cert, const std::string &privkey,
                            cmd::inet_family family)
    : context{nullptr}, sock_fd{-1}, port{-1}, family{family}
{
    context = cmd::ssl_manager::get_server_context();
    SSL_CTX_set_ecdh_auto(context, 1);

    /* Set the key and cert */
    if (SSL_CTX_use_certificate_file(context, cert.c_str(), SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(context);
        throw cmd::ssl_exception("Error using certificate: " + cert);
    }

    if (SSL_CTX_use_PrivateKey_file(context, privkey.c_str(), SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(context);
        throw cmd::ssl_exception("Error using private key: " + privkey);
    }
}

cmd::tls_server::~tls_server()
{
    close();
    if (context)
        SSL_CTX_free(context);
}

cmd::socket::ptr cmd::tls_server::accept()
{
    int client_fd = ::accept(sock_fd, nullptr, nullptr);
    if (client_fd == -1)
        throw cmd::socket_exception("Accept failed: " + std::string(std::strerror(errno)));

    SSL *ssl = SSL_new(context);
    SSL_set_fd(ssl, client_fd);

    if (SSL_accept(ssl) <= 0) {
        ERR_print_errors_fp(stderr);
        throw cmd::ssl_exception("Could not complete handshake");
    }
    return std::make_shared<cmd::tls_socket>(client_fd, ssl);
}

void cmd::tls_server::bind(int port)
{
    if (sock_fd >= 0)
        return;

    sock_fd = bind_server_socket(port, family);
    this->port = port;
}

void cmd::tls_server::close()
{
    if (sock_fd >= 0)
        ::close(sock_fd);
    sock_fd = -1;
}

void cmd::tls_server::listen(int waiting)
{
    int ret = ::listen(sock_fd, waiting);
    if (ret != 0)
        throw cmd::socket_exception(std::strerror(errno));
}

int cmd::tls_server::get_port()
{
    return port;
}
