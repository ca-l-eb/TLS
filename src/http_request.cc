#include <regex>

#include "exceptions.h"
#include "http_pool.h"
#include "http_request.h"
#include "tcp_socket.h"
#include "tls_socket.h"
#include "resource_parser.h"

cmd::http_request::http_request(const std::string &url) : stream{nullptr}, connected{false} {
    std::string proto, host;
    int port;
    std::tie(proto, host, port, resource) = cmd::resource_parser::parse(url);
    bool secure = port == 443 || proto == "https";
    stream = &cmd::http_pool::get_connection(host, port, secure);
    set_header("Host", host);
}

cmd::http_request::http_request(cmd::stream &stream)
    : request_method{"GET"}, resource{"/"}, stream{&stream}, connected{false}
{
    // Set Host header because most servers require it
    set_header("Host", stream.get_sock()->get_host());
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
    // Only allow a single connect() per http_response object
    if (connected)
        return;

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
    auto wrote = stream->write(msg);
    if (wrote != msg.size())
        throw cmd::http_request_exception("Could not send entire HTTP request");
    // Mark this request as connected and read the response
    connected = true;
    resp = cmd::http_response{*stream};
}

cmd::http_response cmd::http_request::response()
{
    return resp;
}

void cmd::http_request::set_resource(const std::string &resource)
{
    this->resource = resource;
}
