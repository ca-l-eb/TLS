#ifndef CMD_BASE64_H
#define CMD_BASE64_H

#include <string>
#include <vector>

namespace cmd
{
namespace base64
{
std::string encode(const char *message, size_t size);
std::string encode(const std::string &message);
std::vector<unsigned char> decode(const char *message, size_t size);
std::vector<unsigned char> decode(const std::string &message);
}
}

#endif
