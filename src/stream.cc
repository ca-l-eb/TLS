#include <cstring>

#include "stream.h"
#include "tokenizer.h"

cmd::stream::stream(cmd::socket::ptr sock) : sock{sock}, remaining_in_buffer{0}
{
    if (sock == nullptr)
        throw std::runtime_error("nullptr passed to cmd::stream");
}

std::string cmd::stream::next_line()
{
    std::string line;
    next_line(line);
    return line;
}

size_t cmd::stream::next_line(std::string &line)
{
    size_t start = line.size();
    cmd::tokenizer::token tok;
    // has_more buffers data and resets buf_ptr and remaining_in_buffer
    while (has_more()) {
        auto remaining = static_cast<size_t>(remaining_in_buffer);
        char *next = cmd::tokenizer::get_line(buf_ptr, remaining, line, tok);
        remaining_in_buffer -= next - buf_ptr;
        buf_ptr = next;
        if (tok == cmd::tokenizer::token::WANT_MORE)
            continue;

        break;
    }
    return line.size() - start;
}

std::string cmd::stream::read(size_t amount)
{
    std::string s;
    read(s, amount);
    return s;
}

size_t cmd::stream::read(std::string &s, size_t amount)
{
    size_t start = s.size();
    while (has_more()) {
        auto remaining = static_cast<size_t>(remaining_in_buffer);
        if (remaining >= amount) {
            s.append(buf_ptr, amount);
            remaining_in_buffer -= amount;
            buf_ptr += amount;
            break;
        } else {
            s.append(buf_ptr, remaining);
            amount -= remaining_in_buffer;
            remaining_in_buffer = 0;
        }
    }
    return s.size() - start;
}

size_t cmd::stream::read(void *buf, size_t amount)
{
    size_t read = 0;
    while (has_more()) {
        auto remaining = static_cast<size_t>(remaining_in_buffer);
        if (remaining >= amount) {
            std::memcpy(buf, buf_ptr, amount);
            remaining_in_buffer -= amount;
            buf_ptr += amount;
            read += amount;
            break;
        } else {
            std::memcpy(buf, buf_ptr, remaining);
            amount -= remaining_in_buffer;
            read += remaining_in_buffer;
            remaining_in_buffer = 0;
        }
    }
    return read;
}

size_t cmd::stream::write(const void *buf, size_t amount)
{
    auto wrote = sock->send(buf, amount, 0);
    if (wrote < 0)
        return 0;
    return static_cast<size_t>(wrote);
}

size_t cmd::stream::write(const std::string &s)
{
    return write(s.c_str(), s.size());
}

void cmd::stream::buffer_data()
{
    // Only buffer data if there is no more in buffer
    if (remaining_in_buffer == 0) {
        remaining_in_buffer = sock->recv(buffer, sizeof(buffer), 0);
        buf_ptr = buffer;  // Reset buf_ptr
    }
}

bool cmd::stream::has_more()
{
    buffer_data();
    return remaining_in_buffer > 0;
}

cmd::socket *cmd::stream::get_sock()
{
    return sock.get();
}
