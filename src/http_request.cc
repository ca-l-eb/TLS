#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "http_request.h"
#include "plain_socket.h"
#include "tls_socket.h"
#include "tokenizer.h"

cmd::http_request::http_request(const std::string &url, SSL_CTX *context) : code {0}, request {"GET"} {
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

    set_header("Host", host);

    if (proto == "http") {
        sock = std::make_shared<cmd::plain_socket>();
        port = 80;
    }
    else if (proto == "https") {
        sock = std::make_shared<cmd::tls_socket>(context);
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

    cmd::line_token tok;
    cmd::tokenizer t;

    std::vector<std::string> headers_vector;
    std::string headers, body;
    std::string line;

    char buffer[2048];
    bool header_end = false;
    try {
        int recv;
        while ((recv = sock->recv(buffer, sizeof(buffer))) > 0) {
            std::cout << "read " << recv << " bytes\n";
            int remaining = recv;
            char *buf_ptr = buffer; 
            while (remaining > 0) {
                char *next = t.get_line(buf_ptr, remaining, line, tok);
                if (tok == cmd::line_token::COMPLETE) {
                    if (line.length() == 0) {
                        if (!header_end) {
                            header_end = true;
                            remaining -= next - buf_ptr;
                            buf_ptr = next;
                            continue;
                        }
                    }
                    if (!header_end) {
                        // HTTP Headers
                        headers_vector.push_back(line);
                        headers += line + "\n";
                    }
                    else {
                        // HTTP Body
                        body += line + "\n";
                    }
                    line.clear();
                }
                remaining -= next - buf_ptr;
                buf_ptr = next;
            }
        }
        if (line.length() != 0)
            body += line + "\n";
        std::cout << "\n---------------Headers-------------\n" << headers;
        std::cout << "------------------------------------\n";

        std::cout << "\n-------------HTTP BODY------------\n" << body;
        std::cout << "-----------------------------------\n";

    }
    catch (std::exception &e) {
        std::cerr << e.what() << "\n";
    }
}
