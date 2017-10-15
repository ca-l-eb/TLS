#include <regex>

#include "http_pool.h"
#include "http_request.h"
#include "tcp_socket.h"
#include "tls_socket.h"

cmd::http_request::http_request(cmd::stream &stream)
    : request_method{"GET"}, resource{"/"}, stream{stream}
{
    host = stream.get_sock()->get_host();
    // Set Host header because most servers require it
    set_header("Host", host);
}

void cmd::http_request::set_header(const std::string &header, const std::string &value)
{
    headers[header] = value;
}

void cmd::http_request::set_body(const std::string &body)
{
    this->body = body;
}

void cmd::http_request::set_request_method(const std::string &request)
{
    this->request_method = request;
}

void cmd::http_request::connect()
{
    std::string msg;
    msg += request_method + " " + resource + " HTTP/1.1\r\n";
    for (auto &header : headers) {
        msg += header.first + ": " + header.second + "\r\n";
    }
    if (body.length() > 0) {
        msg += "Content-Length: " + std::to_string(body.length());
        msg += "\r\n\r\n";
        msg += body;
    } else {
        msg += "\r\n";
    }
    stream.get_sock()->send(msg, 0);
    headers.clear();           // Clear headers for next connect()
    set_header("Host", host);  // Reset Host
    resource = "/";            // Default resource
    body = "";                 // Clear body
}

cmd::http_response cmd::http_request::response()
{
    return http_response{stream};
}

void cmd::http_request::set_resource(const std::string &resource)
{
    this->resource = resource;
}
