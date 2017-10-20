#ifndef CMD_STREAM_H
#define CMD_STREAM_H

#include <string>

#include "socket.h"

namespace cmd
{
class stream
{
public:
    stream();
    explicit stream(cmd::socket::ptr sock);

    // Return next line in stream
    std::string next_line();

    // Return number of bytes in next line (written to line)
    size_t next_line(std::string &line);

    // Read up to amount bytes into the buffer, return amount bytes actually read
    size_t read(void *buf, size_t amount);
    size_t read(std::string &s, size_t amount);
    std::string read(size_t amount);

    size_t write(const void *buf, size_t amount);
    size_t write(const std::string &s);

    bool has_more();
    cmd::socket *get_sock();

private:
    cmd::socket::ptr sock;

    std::vector<char> buffer;
    int buf_idx;
    ssize_t remaining_in_buffer;

    void buffer_data();
};
}  // namespace cmd

#endif
