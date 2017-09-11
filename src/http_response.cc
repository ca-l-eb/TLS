#include <regex>

#include "http_response.h"
#include "stream.h"

cmd::http_response::http_response(cmd::stream &stream) :
    status {0}
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
    return body;
}

std::vector<std::string> cmd::http_response::headers()
{
    return headers_list;
}


void cmd:http_response::read_response(cmd::stream &stream) {
    std::string line;

    // Read until we get empty line -- i.e. end of headers section
    while (true) {
        line.clear();
        stream.next_line(line);
        if (line.length() == 0)
            break;
        headers_list.push_back(line);
    }
    process_headers();

    std::cout << "\n-------------HTTP BODY------------\n" << body;
    std::cout << "-----------------------------------\n";

}

void cmd::http_response::process_headers() {
    if (headers.size() < 1)
        throw std::runtime_error("Invalid HTTP format");

    auto it = headers.begin();
    std::regex re {"^HTTP/(\\S+) (\\d{3}) (.*)$"};
    std::smatch matcher;
    std::regex_search(*it, matcher, re);
    if (matcher.size() > 0) {
        std::string http_version = matcher.str(1);
        status = std::stoi(matcher.str(2));
        status_message_str = matcher.str(3);
    }
    else
        throw std::runtime_error("Invalid HTTP response");

    // Process headers
    ++it;
    re = std::regex{"^(\\S+):\\s*(.*)$"};
    for (; it != headers.end(); ++it) {
        std::regex_search(*it, matcher, re); 
        if (matcher.size() > 0) {
            std::string key = matcher.str(1);
            std::string val = matcher.str(2);
            response_headers[key] = val;
            std::cout << key << "(" << val << ")\n";
        }
        else {
            std::cerr << "Bad header: " << *it << "\n";
        }
    }
}
