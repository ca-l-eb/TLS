#include <regex>

#include "exceptions.h"
#include "http_pool.h"
#include "http_response.h"
#include "string_utils.h"

cmd::http_response::http_response() : status{0} {}

cmd::http_response::http_response(cmd::stream &stream) : status{0}, length{0}, type{body_type::NONE}
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

std::multimap<std::string, std::string> &cmd::http_response::headers()
{
    return headers_map;
}

std::string cmd::http_response::version()
{
    return http_version;
}

void cmd::http_response::read_response(cmd::stream &stream)
{
    std::string line;

    // First line should be the HTTP response status line
    stream.next_line(line);
    check_response_status(line);

    // Read until we get empty line -- i.e. end of headers section
    while (true) {
        line.clear();
        stream.next_line(line);
        if (line.length() == 0)
            break;

        add_header_to_map(line);
    }
    process_headers(stream);

    if (status == 204)
        return;  // No content

    switch (type) {
        case body_type::NONE:
            // No content
            break;
        case body_type::CHUNKED:
            do_chunked(stream);
            break;
        case body_type::LENGTH:
            do_content_length(stream);
            break;
        case body_type::ALL:
            do_read_all(stream);
            break;
    }
}

void cmd::http_response::process_headers(cmd::stream &s)
{
    check_connection_close(s);  // Note: must be called before check_body_method
    check_body_method();
}

void cmd::http_response::check_response_status(const std::string &status_line)
{
    static std::regex re{R"(^(HTTP/\S+) (\d{3}) (.*)$)"};

    std::smatch matcher;
    std::regex_search(status_line, matcher, re);
    if (matcher.empty())
        throw cmd::http_response_exception("Invalid HTTP response status line");

    http_version = matcher.str(1);
    status = std::stoi(matcher.str(2));
    status_message_str = matcher.str(3);
}

void cmd::http_response::add_header_to_map(const std::string &line)
{
    static std::regex re{R"(^(\S+):\s*(.*)$)"};
    std::smatch matcher;
    std::regex_search(line, matcher, re);
    if (!matcher.empty()) {
        std::string first = cmd::string_utils::to_lower(matcher.str(1));
        auto pair = std::pair<std::string, std::string>(first, matcher.str(2));
        headers_map.insert(pair);
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
    auto range = headers_map.equal_range("content-length");
    for (auto it = range.first; it != range.second; ++it) {
        auto len = static_cast<size_t>(std::stoi(it->second));
        if (len != length) {
            if (type == body_type::LENGTH) {
                throw cmd::http_response_exception("Got conflicting Content-Length headers");
            }
        }
        length = len;
        type = body_type::LENGTH;
    }
}

void cmd::http_response::check_transfer_encoding()
{
    auto range = headers_map.equal_range("transfer-encoding");
    for (auto it = range.first; it != range.second; ++it) {
        if (it->second.find("chunked") != std::string::npos)
            type = body_type::CHUNKED;
    }
    if (type == body_type::CHUNKED) {
        // TODO: check for Trailer header to add only listed trailing headers
    }
}

void cmd::http_response::do_chunked(cmd::stream &s)
{
    std::string line;
    while (true) {
        line.clear();
        s.next_line(line);
        length = static_cast<size_t>(std::stoi(line, nullptr, 16));  // Convert hex chunk length
        if (length == 0)
            break;
        ssize_t read = s.read(body_str, length);
        if (read != static_cast<ssize_t>(length))
            throw cmd::http_response_exception("Could not read entire chunk");
        line.clear();
        s.next_line(line);
        if (line.length() != 0)
            throw cmd::http_response_exception(
                "Got invalid chunk terminator. Expected CRLF but got: " + line);
    }

    // Check trailing headers
    while (true) {
        line.clear();
        s.next_line(line);
        if (line.length() == 0)  // Empty line -> DONE
            break;
        // Add any trailing headers to the headers map
        add_header_to_map(line);
    }
}

void cmd::http_response::do_content_length(cmd::stream &s)
{
    auto read = s.read(body_str, length);
    if (read != length) {
        std::string err_msg = "Could not read entire Content-Length. Got " + std::to_string(read) +
                              " bytes but expected " + std::to_string(length);
        throw cmd::http_response_exception(err_msg);
    }
}

void cmd::http_response::do_read_all(cmd::stream &s)
{
    while (s.has_more()) {
        s.read(body_str, 256 * 1024);  // Large amount to reduce number of calls
    }
}

void cmd::http_response::check_connection_close(cmd::stream &s)
{
    auto range = headers_map.equal_range("connection");
    for (auto it = range.first; it != range.second; ++it) {
        if (it->second.find("close") != std::string::npos) {
            cmd::socket *sock = s.get_sock();
            std::string host = sock->get_host();
            int port = sock->get_port();
            cmd::http_pool::mark_closed(host, port);

            // Read all by default if connection closed, will be overwritten
            // by check_body_method if it finds more specific details for body format
            type = body_type::ALL;
        }
    }
}
