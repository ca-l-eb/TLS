#ifndef CMD_SSL_MANAGER_H
#define CMD_SSL_MANAGER_H

#include <openssl/ssl.h>

namespace cmd
{
class ssl_manager
{
public:
    static SSL_CTX *get_context();
    static SSL_CTX *get_server_context();

private:
    ssl_manager();
    ~ssl_manager();
    static ssl_manager &instance()
    {
        static ssl_manager instance;
        return instance;
    }

    SSL_CTX *context;
    const SSL_METHOD *method;
};
};

#endif
