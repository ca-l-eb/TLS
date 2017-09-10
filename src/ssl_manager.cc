#include <memory>
#include <stdexcept>
#include <cassert>
#include <iostream>

#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>
#include <openssl/err.h>

#include "ssl_manager.h"

static void print_error_info() {
    int error = ERR_get_error();
    if (error != 0)
        std::cerr << ERR_error_string(error, NULL) << "\n";
}

cmd::ssl_manager::ssl_manager() {
    SSL_load_error_strings();
    ERR_load_crypto_strings();
    SSL_library_init();

    int error = ERR_get_error();
    assert (error == 0);

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

    // Use default certificate store (/etc/ssl/cert.pem)
    // and directory chain (/etc/ssl/certs/)
    int ret = SSL_CTX_set_default_verify_paths(context);
    if (ret != 1) {
        print_error_info();
        throw std::runtime_error("Could not load default CA");
    }
}

cmd::ssl_manager::~ssl_manager() {
    SSL_CTX_free(context);
    ERR_free_strings();
}

SSL_CTX *cmd::ssl_manager::get_context() const {
    return context;
}
