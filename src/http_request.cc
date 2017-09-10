#include <iostream>
#include <sstream>
#include <string>

#include "http_request.h"
#include "plain_socket.h"
#include "tls_socket.h"

cmd::http_request::http_request(const std::string &url) : code {0}, request {"GET"} {
    // Extract host and uri element from url
    std::size_t proto_end = url.find("://");
    if (proto_end == std::string::npos)
        throw std::runtime_error("Could not find protocol in url: " + url);

    std::string proto = url.substr(0, proto_end);
    
    std::size_t host_end = url.find("/", proto_end+3);
    host = url.substr(proto_end+3, host_end-(proto_end+3));

    resource = "/";
    if (host_end != std::string::npos)
        resource += url.substr(host_end+1, std::string::npos);

    std::cout << "Proto: " << proto << "\n";
    std::cout << "Host: " << host << "\n";
    std::cout << "Resource: " << resource << "\n";

    if (proto == "http") {
        sock = std::unique_ptr<cmd::socket>(new cmd::plain_socket);
        port = 80;
    }
    else if (proto == "https") {
        sock = std::unique_ptr<cmd::socket>(new cmd::tls_socket(nullptr)); // should pass tls config instance
        port = 443;
    }
    else 
        throw std::runtime_error("Unsupported protocol: " + proto);
}

void cmd::http_request::set_header(const std::string &header, const std::string &value) {
    headers[header] = value;
}

void cmd::http_request::set_body(const std::string &body) {
    this->body = body; 
}

void cmd::http_request::set_request_method(const std::string &request) {
    this-> request = request;
}

void cmd::http_request::connect() {
    sock->connect(host, port);
    std::string msg;
    msg += request + " " + resource + " HTTP/1.1\r\n";
    for (auto it = headers.begin(); it != headers.end(); ++it) {
        msg += it->first + ": " + it->second + "\r\n";
    }
    msg += "\n\r";
    if (body != "")
        msg += body + "\r\n";

    sock->send(msg);
}

std::string cmd::http_request::response() {
    read_response();
    return response_str;
}

int cmd::http_request::response_code() {
    read_response();
    return code;
}

void cmd::http_request::read_response() {
    if (code != 0)
        return;

    std::stringstream ss;
    char buffer[2048];
    do {
        int len = sock->recv(buffer, sizeof(buffer)-1, 0); 
        if (len == 0)
            break;
        buffer[len] = '\0';
        ss << buffer;
    } while (true);
}
