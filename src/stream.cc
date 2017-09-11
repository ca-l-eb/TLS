#include <cstring>

#include "tokenizer.h"
#include "stream.h"

cmd::stream::stream(cmd::socket::ptr sock) :
    sock {sock},
    remaining_in_buffer {0}
{
}

std::string cmd::stream::next_line()
{
    std::string line;
    next_line(line);
    return line;
}

int cmd::stream::next_line(std::string &line)
{
    int start = line.size();
    cmd::line_token token;
    cmd::tokenizer tokenizer;
    // has_more buffers data and resets buf_ptr and remaining_in_buffer
    while (has_more()) {
        char *next = tokenizer.get_line(buf_ptr, remaining_in_buffer, line, token);
        if (token == cmd::line_token::WANT_MORE)
            continue;

        remaining_in_buffer -= next - buf_ptr;
        buf_ptr = next;
        break;
    }
    return line.size() - start;
}

std::string cmd::stream::read(int amount)
{
    std::string s;
    read(s, amount);
    return s;
}

int cmd::stream::read(std::string &s, int amount)
{
    int start = s.size();
    while (has_more()) {
        if (remaining_in_buffer >= amount) {
            s.append(buf_ptr, amount);
            remaining_in_buffer -= amount;
            buf_ptr += amount;
            break;
        }
        else {
            s.append(buf_ptr, remaining_in_buffer);
            amount -= remaining_in_buffer;
            remaining_in_buffer = 0;
        }
    }
    return s.size() - start;
}

int cmd::stream::read(char *buf, int amount)
{
    int read = 0;
    while (has_more()) {
        if (remaining_in_buffer >= amount) {
            std::memcpy(buf, buf_ptr, amount);
            remaining_in_buffer -= amount;
            buf_ptr += amount;
            break;
        }
        else {
            std::memcpy(buf, buf_ptr, remaining_in_buffer);
            amount -= remaining_in_buffer;
            remaining_in_buffer = 0;
        }
    }
    return read;
}

void cmd::stream::buffer_data() 
{
    // Only buffer data if there is no more in buffer
    if (remaining_in_buffer == 0) {
        remaining_in_buffer = sock->recv(buffer, sizeof(buffer));
        buf_ptr = buffer; // Reset buf_ptr
    }
}

bool cmd::stream::has_more()
{
    buffer_data();
    return remaining_in_buffer > 0;
}
