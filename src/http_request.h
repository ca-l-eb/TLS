#ifndef CMD_HTTP_REQUEST_H
#define CMD_HTTP_REQUEST_H

#include <string>
#include <memory>
#include <map>

#include "socket.h"

namespace cmd {

class http_request {
public:
    http_request(const std::string &url);
    void set_request_method(const std::string &method);
    void set_header(const std::string &header, const std::string &value);
    void set_body(const std::string &body);
    void connect();
    int response_code();
    std::string response();

private:
    std::unique_ptr<cmd::socket> sock;
    std::string host, request, resource, body, response_str;
    std::map<std::string, std::string> headers;
    int port, code;

    void read_response();
};

};

#endif
