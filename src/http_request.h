#ifndef CMD_HTTP_REQUEST_H
#define CMD_HTTP_REQUEST_H

#include <string>
#include <vector>
#include <memory>
#include <map>

#include <openssl/ssl.h>

#include "socket.h"
#include "http_response.h"

namespace cmd {

class http_request {
public:
    http_request(const std::string &url, SSL_CTX *context);
    void set_request_method(const std::string &method);
    void set_header(const std::string &header, const std::string &value);
    void set_body(const std::string &body);
    void connect();
    cmd::http_response response();

private:
    cmd::socket::ptr sock;
    // Sending information
    std::string host, request_method, resource, body;
    std::map<std::string, std::string> headers;

    int port;
    void setup_socket(const std::string &proto, SSL_CTX *context);
};

};

#endif
