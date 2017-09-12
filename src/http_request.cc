#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "http_request.h"
#include "plain_socket.h"
#include "stream.h"
#include "tls_socket.h"

cmd::http_request::http_request(const std::string &url, SSL_CTX *context) : request_method{"GET"}
{
    // Extract host and uri element from url
    std::size_t proto_end = url.find("://");
    if (proto_end == std::string::npos)
        throw std::runtime_error("Could not find protocol in url: " + url);

    std::string proto = url.substr(0, proto_end);

    std::size_t offset = proto_end + 3;  // Skip ://
    std::size_t host_end = url.find("/", offset);
    host = url.substr(offset, host_end - offset);

    resource = "/";
    if (host_end != std::string::npos)
        resource += url.substr(host_end + 1, std::string::npos);

    // Set Host header because many servers require it
    set_header("Host", host);

    // Use the protocol to create a socket
    setup_socket(proto, context);
}

void cmd::http_request::setup_socket(const std::string &proto, SSL_CTX *context)
{
    if (proto == "http") {
        sock = std::make_shared<cmd::plain_socket>();
        port = 80;
    } else if (proto == "https") {
        sock = std::make_shared<cmd::tls_socket>(context);
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
}

cmd::http_response cmd::http_request::response()
{
    cmd::stream s{sock};
    cmd::http_response r{s};
    return r;
}
