#include <regex>
#include <stdexcept>
#include <string>
#include <vector>

#include "http_pool.h"
#include "http_request.h"
#include "stream.h"
#include "tcp_socket.h"
#include "tls_socket.h"

static std::regex url_re{
    "^(\\S+)://([A-Za-z0-9.-]{2,})(?::(\\d+))?(/[/A-Za-z0-9-._~:/?#\\[\\]@!$&'()*+,;=`]*)?$"};

cmd::http_request::http_request(const std::string &url)
    : request_method{"GET"}, resource{"/"}, port{-1}, retries{0}
{
    std::smatch matcher;
    std::regex_match(url, matcher, url_re);

    std::string proto;
    if (matcher.size() > 0) {
        proto = matcher.str(1);
        host = matcher.str(2);
        if (matcher.str(3) != "") {
            port = std::stoi(matcher.str(3));
        } else {
            if (proto == "http")
                port = 80;
            else if (proto == "https")
                port = 443;
            else
                throw std::runtime_error("Unsupported protocol: " + proto);
        }
        if (matcher.str(4) != "")
            resource = matcher.str(4);
    } else
        throw std::runtime_error("Invalid url: " + url);

    // Set Host header because many servers require it
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
    try {
        sock = cmd::http_pool::get_connection(host, port, port == 443);
        std::string msg;
        msg += request_method + " " + resource + " HTTP/1.1\r\n";
        for (auto it = headers.begin(); it != headers.end(); ++it) {
            msg += it->first + ": " + it->second + "\r\n";
        }
        if (body.length() > 0) {
            msg += "Content-Length: " + std::to_string(body.length());
            msg += "\r\n\r\n";
            msg += body;
        } else {
            msg += "\r\n";
        }
        sock->send(msg);
        headers.clear();           // Clear headers for next connect()
        set_header("Host", host);  // Reset Host
        resource = "/";            // Default resource
        body = "";                 // Clear body
        retries = 0;               // Success, reset retries
    } catch (std::exception &e) {
        cmd::http_pool::mark_closed(host, port);
        retries++;
        if (retries == 2)
            throw;  // Give up after 2 failed attempts

        // Connect again; connection might have been auto closed for being open too long without use
        connect();
    }
}

cmd::http_response cmd::http_request::response()
{
    // Make a shared stream to be used for multiple requests
    if (stream.get() == nullptr)
        stream = std::unique_ptr<cmd::stream>(new cmd::stream{sock});
    http_response response{*stream};
    return response;
}

void cmd::http_request::set_resource(const std::string &resource)
{
    this->resource = resource;
}
