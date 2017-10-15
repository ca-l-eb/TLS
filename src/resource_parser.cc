#include <regex>
#include <string>

#include "resource_parser.h"

void parse(const std::string &url)
{
    static std::regex re{
        R"(^(\S+)://([A-Za-z0-9.-]{2,})(?::(\d+))?(/[/A-Za-z0-9-._~:/?#\[\]@!$&'()*+,;=`]*)?$)"};
    std::smatch matcher;
    std::regex_match(url, matcher, re);

    std::string host{};
    int port;
    std::string proto{};
    std::string resource{};
    if (matcher.empty())
        throw std::runtime_error("Invalid url: " + url);

    proto = matcher.str(1);
    host = matcher.str(2);
    if (!matcher.str(3).empty()) {
        port = std::stoi(matcher.str(3));
    } else {
        if (proto == "http" || proto == "ws")
            port = 80;
        else if (proto == "https" || proto == "wss")
            port = 443;
        else
            throw std::runtime_error("Unsupported protocol: " + proto);
    }
    if (!matcher.str(4).empty())
        resource = matcher.str(4);
}