#ifndef CMD_RESOURCE_PARSER_H
#define CMD_RESOURCE_PARSER_H

#include "socket.h"

namespace cmd
{
namespace resource_parser{
    std::tuple<std::string, std::string, int, std::string> parse(const std::string &url);
}
}

#endif
