#include <iostream>
#include <regex>
#include <stdexcept>
#include <string>
#include <vector>

#include "http_request.h"
#include "plain_socket.h"
#include "stream.h"
#include "tls_socket.h"

cmd::http_request::http_request(const std::string &url, SSL_CTX *context)
    : request_method{"GET"}, resource{"/"}, port{-1}
{
    std::regex re{"^(https?)://([A-Za-z0-9.-]{2,})(?::(\\d+))?(/[/A-Za-z0-9$-_.+!*()#%&?<>]*)?$"};
    std::smatch matcher;
    std::regex_match(url, matcher, re);

    std::string proto;
    if (matcher.size() > 0) {
        proto = matcher.str(1);
        host = matcher.str(2);
        if (matcher.str(3) != "")
            port = std::stoi(matcher.str(3));
        if (matcher.str(4) != "")
            resource = matcher.str(4);
    } else
        throw std::runtime_error("Invalid url: " + url);

    // Set Host header because many servers require it
    set_header("Host", host);

    // Use the protocol to create a socket
    setup_socket(proto, context);
}

void cmd::http_request::setup_socket(const std::string &proto, SSL_CTX *context)
{
    if (proto == "http") {
        sock = std::make_shared<cmd::plain_socket>();
        if (port == -1)  // Use default port 80 if not specified
            port = 80;
    } else if (proto == "https") {
        sock = std::make_shared<cmd::tls_socket>(context);
        if (port == -1)
            port = 443;
    } else
        throw std::runtime_error("Unsupported protocol: " + proto);
}

void cmd::http_request::set_header(const std::string &header, const std::string &value)
{
    headers[header] = value;
}

void cmd::http_request::set_body(const std::string &body) { this->body = body; }

void cmd::http_request::set_request_method(const std::string &request)
{
    this->request_method = request;
}

void cmd::http_request::connect()
{
    sock->connect(host, port);
    std::string msg;
    msg += request_method + " " + resource + " HTTP/1.1\r\n";
    for (auto it = headers.begin(); it != headers.end(); ++it) {
        msg += it->first + ": " + it->second + "\r\n";
    }
    msg += "\n\r";
    if (body != "")
        msg += body;

    sock->send(msg);

    headers.clear();           // Clear headers for next connect()
    set_header("Host", host);  // Reset Host
    resource = "/";            // Default resource
}

cmd::http_response cmd::http_request::response()
{
    cmd::stream s{sock};
    cmd::http_response r{s};
    return r;
}

void cmd::http_request::set_resource(const std::string &resource) { this->resource = resource; }
