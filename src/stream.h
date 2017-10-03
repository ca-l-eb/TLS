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
    int next_line(std::string &line);

    // Read amount bytes into the buffer, return amount bytes actually written
    int read(void *buf, int amount);
    int read(std::string &s, int amount);
    std::string read(int amount);

    bool has_more();
    cmd::socket *get_sock()
    {
        return sock.get();
    }

private:
    cmd::socket::ptr sock;

    char buffer[4096];
    char *buf_ptr;
    int remaining_in_buffer;

    void buffer_data();
};
}  // namespace cmd

#endif
