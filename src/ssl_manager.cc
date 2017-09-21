#include <cassert>
#include <iostream>
#include <memory>
#include <stdexcept>

#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>

#include "ssl_manager.h"
#include "tls_socket.h"

static void print_error_info()
{
    int error = ERR_get_error();
    if (error != 0)
        std::cerr << ERR_error_string(error, NULL) << "\n";
}

cmd::ssl_manager::ssl_manager()
{
    SSL_load_error_strings();
    ERR_load_crypto_strings();
    SSL_library_init();

    print_error_info();

    method = TLS_method();
    if (method == NULL) {
        print_error_info();
        throw std::runtime_error("Error creating TLS method");
    }

    context = SSL_CTX_new(method);
    if (context == NULL) {
        print_error_info();
        throw std::runtime_error("Could not create OpenSSL context");
    }

    SSL_CTX_set_verify_depth(context, 4);
    print_error_info();

    const long flags = SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_COMPRESSION;
    SSL_CTX_set_options(context, flags);
    print_error_info();

    // Use default certificate store and directory chain
    // (/etc/ssl/cert.pem and /etc/ssl/certs/ on Arch Linux)
    int ret = SSL_CTX_set_default_verify_paths(context);
    if (ret != 1) {
        print_error_info();
        throw std::runtime_error("Could not load default CA");
    }
}

cmd::ssl_manager::~ssl_manager()
{
    SSL_CTX_free(context);
    ERR_free_strings();
}

SSL_CTX *cmd::ssl_manager::get_context() const
{
    return context;
}

cmd::socket::ptr cmd::ssl_manager::get_socket_ptr()
{
    return std::make_shared<cmd::tls_socket>(context);
}
