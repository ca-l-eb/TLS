#ifndef CMD_STREAM_H
#define CMD_STREAM_H

#include <string>

#include "socket.h"

namespace cmd
{
class stream
{
public:
    stream(cmd::socket::ptr sock);

    // Return next line in stream
    std::string next_line();

    // Return number of bytes in next line (written to line)
    size_t next_line(std::string &line);

    // Read amount bytes into the buffer, return amount bytes actually written
    size_t read(void *buf, size_t amount);
    size_t read(std::string &s, size_t amount);
    std::string read(size_t amount);

    bool has_more();
    cmd::socket *get_sock();

private:
    cmd::socket::ptr sock;

    char buffer[4096];
    char *buf_ptr;
    ssize_t remaining_in_buffer;

    void buffer_data();
};
}  // namespace cmd

#endif
