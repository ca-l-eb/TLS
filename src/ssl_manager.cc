#include <cassert>
#include <memory>

#include <openssl/err.h>
#include <openssl/ssl.h>

#include "ssl_manager.h"
#include "tls_socket.h"

static void throw_error_info(const std::string &msg)
{
    auto error = ERR_get_error();
    if (error != 0) {
        std::string error_string = std::string(ERR_error_string(error, nullptr));
        throw std::runtime_error(msg + "::" + error_string);
    }
}

static const long flags = SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_COMPRESSION;

cmd::ssl_manager::ssl_manager()
{
    SSL_load_error_strings();
    ERR_load_crypto_strings();
    SSL_library_init();

#if (OPENSSL_VERSION_NUMBER >= 0x10100000L)
    method = TLS_client_method();
#else
    method = SSLv23_client_method();
#endif
    if (method == nullptr)
        throw_error_info("Error creating TLS method");

    context = SSL_CTX_new(method);
    if (context == nullptr)
        throw_error_info("Could not create OpenSSL context");

    SSL_CTX_set_verify_depth(context, 4);
    SSL_CTX_set_options(context, flags);

    // Use default certificate store and directory chain
    // (/etc/ssl/cert.pem and /etc/ssl/certs/ on Arch Linux)
    int ret = SSL_CTX_set_default_verify_paths(context);
    if (ret != 1)
        throw_error_info("Could not load default CA");
}

cmd::ssl_manager::~ssl_manager()
{
    EVP_cleanup();
    ERR_free_strings();
    SSL_CTX_free(context);
}

SSL_CTX *cmd::ssl_manager::get_context()
{
    return instance().context;
}

SSL_CTX *cmd::ssl_manager::get_server_context()
{
    // Make sure global OpenSSL state is loaded
    instance();

    const SSL_METHOD *method;
#if (OPENSSL_VERSION_NUMBER >= 0x10100000L)
    method = TLS_server_method();
#else
    method = SSLv23_client_method();
#endif
    if (method == nullptr)
        throw_error_info("Could not make TLS server method for OpenSSL");

    SSL_CTX *context = SSL_CTX_new(method);
    if (context == nullptr)
        throw_error_info("Could not make new SSL server context with TLS method");

    SSL_CTX_set_options(context, flags);
    return context;
}
