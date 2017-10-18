#ifndef CMD_EXCEPTIONS_H
#define CMD_EXCEPTIONS_H

#include <stdexcept>

namespace cmd
{
struct http_request_exception : public std::runtime_error {
    explicit http_request_exception(const std::string &__arg) : runtime_error(__arg) {}
};
struct http_response_exception : public std::runtime_error {
    explicit http_response_exception(const std::string &__arg) : runtime_error(__arg) {}
};

// WebSocket protocol exception, invalid bits, protocol validations, etc.
struct websocket_exception : public std::runtime_error {
    explicit websocket_exception(const std::string &__arg) : runtime_error(__arg) {}
};

// Generic socket exception, e.g. read, write, accept, listen errors
struct socket_exception : public std::runtime_error {
    explicit socket_exception(const std::string &__arg) : runtime_error(__arg) {}
};

// Exceptions relating to OpenSSL, e.g. handshake error, X509 certificate validation error, etc.
struct ssl_exception : public std::runtime_error {
    explicit ssl_exception(const std::string &__arg) : runtime_error(__arg) {}
};
}

#endif
