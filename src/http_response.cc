#include <regex>

#include "http_pool.h"
#include "http_response.h"
#include "stream.h"

static bool contains(const std::string &str, const std::string &c)
{
    return str.find(c) != str.npos;
}

cmd::http_response::http_response(cmd::stream &stream) : status{0}, length{0}, type{NONE}
{
    read_response(stream);
}

int cmd::http_response::status_code()
{
    return status;
}

std::string cmd::http_response::status_message()
{
    return status_message_str;
}

std::string cmd::http_response::body()
{
    return body_str;
}

std::vector<std::string> cmd::http_response::headers()
{
    return headers_list;
}

std::string cmd::http_response::version()
{
    return http_version;
}

void cmd::http_response::read_response(cmd::stream &stream)
{
    std::string line;

    // Read until we get empty line -- i.e. end of headers section
    while (true) {
        line.clear();
        stream.next_line(line);
        if (line.length() == 0)
            break;
        headers_list.push_back(line);
    }
    process_headers(stream);

    if (status == 204)
        return;  // No content

    if (type == CHUNKED)
        do_chunked(stream);
    else if (type == LENGTH)
        do_content_length(stream);
    else
        do_read_all(stream);
}

void cmd::http_response::process_headers(cmd::stream &s)
{
    check_response_code();
    add_headers_to_map();

    check_body_method();
    check_headers(s);
}

static std::regex response_re{"^(HTTP/\\S+) (\\d{3}) (.*)$"};

void cmd::http_response::check_response_code()
{
    if (headers_list.size() < 1)
        throw std::runtime_error("Did not receive HTTP response");

    std::smatch matcher;
    std::regex_search(headers_list[0], matcher, response_re);
    if (matcher.size() > 0) {
        http_version = matcher.str(1);
        status = std::stoi(matcher.str(2));
        status_message_str = matcher.str(3);

        // Remove this from headers_list because it isn't really a header
        headers_list.erase(headers_list.begin());
    } else
        throw std::runtime_error("Invalid HTTP response");
}

void cmd::http_response::add_headers_to_map()
{
    std::regex re = std::regex{"^(\\S+):\\s*(.*)$"};
    std::smatch matcher;

    for (auto it = headers_list.begin(); it != headers_list.end(); ++it) {
        std::regex_search(*it, matcher, re);
        if (matcher.size() > 0) {
            auto pair = std::pair<std::string, std::string>(matcher.str(1), matcher.str(2));
            headers_map.insert(pair);
        } else {
            // Bad header entry, remove from list
            headers_list.erase(it);
        }
    }
}
void cmd::http_response::check_body_method()
{
    check_content_length();
    // Check for Transfer-Encoding second because we always accept chunked data
    // in favor of Content-Length when provided
    check_transfer_encoding();
}

void cmd::http_response::check_content_length()
{
    auto range = headers_map.equal_range("Content-Length");
    for (auto it = range.first; it != range.second; ++it) {
        int len = std::stoi(it->second);
        if (type == LENGTH && len != length) {
            throw std::runtime_error("Got conflicting Content-Length headers");
        }
        length = len;
        type = LENGTH;
    }
}

void cmd::http_response::check_transfer_encoding()
{
    auto range = headers_map.equal_range("Transfer-Encoding");
    for (auto it = range.first; it != range.second; ++it) {
        // Ignoring
        if (contains(it->second, "chunked"))
            type = CHUNKED;
    }
    if (type == CHUNKED) {
        // TODO: check for Trailer header to add only listed trailing headers
    }
}

void cmd::http_response::do_chunked(cmd::stream &s)
{
    std::string line;
    while (true) {
        line.clear();
        s.next_line(line);
        length = std::stoi(line, NULL, 16);  // Convert hex chunk length
        if (length == 0)
            break;
        int read = s.read(body_str, length);
        if (read != length)
            throw std::runtime_error("Got invalid chunk length");
        line.clear();
        s.next_line(line);
        if (line.length() != 0)
            throw std::runtime_error("Got invalid chunk terminator. Expected CRLF but got: " +
                                     line);
    }

    // Check trailing headers
    while (true) {
        line.clear();
        s.next_line(line);
        if (line.length() == 0)  // Empty line -> DONE
            break;
        headers_list.push_back(line);
    }
}

void cmd::http_response::do_content_length(cmd::stream &s)
{
    int read = s.read(body_str, length);
    if (read != length) {
        std::string err_msg = "Could not read entire Content-Length. Got " + std::to_string(read) +
                              " bytes but expected " + std::to_string(length);
        throw std::runtime_error(err_msg);
    }
}

void cmd::http_response::do_read_all(cmd::stream &s)
{
    while (s.has_more()) {
        s.read(body_str, 256 * 1024);  // Large amount to reduce number of calls
    }
}

void cmd::http_response::check_headers(cmd::stream &s)
{
    auto range = headers_map.equal_range("Connection");
    for (auto it = range.first; it != range.second; ++it) {
        if (contains(it->second, "close")) {
            cmd::socket *sock = s.get_sock();
            std::string host = sock->get_host();
            int port = sock->get_port();
            cmd::http_pool::mark_closed(host, port);
        }
    }
}
