#ifndef CMD_SOCKET_H
#define CMD_SOCKET_H

#include <memory>
#include <vector>

namespace cmd
{
class socket
{
public:
    virtual ~socket() = default;
    virtual void connect(const std::string &host, int port) = 0;
    virtual void close() = 0;
    virtual ssize_t send(const void *buffer, size_t size, int flags) = 0;
    virtual ssize_t send(const std::string &str, int flags) = 0;
    virtual ssize_t recv(void *buffer, size_t size, int flags) = 0;
    virtual ssize_t recv(std::vector<unsigned char> &buf, int flags) = 0;
    virtual int get_fd() = 0;
    virtual int get_port() = 0;
    virtual std::string get_host() = 0;

    using ptr = std::shared_ptr<cmd::socket>;
};
}  // namespace cmd

#endif
