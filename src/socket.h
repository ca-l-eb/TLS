#ifndef CMD_SOCKET_H
#define CMD_SOCKET_H

#include <memory>
#include <vector>

namespace cmd
{
class socket
{
public:
    virtual ~socket() {}
    virtual void connect(const std::string &host, int port) = 0;
    virtual void connect(const char *host, int port) = 0;
    virtual void close() = 0;
    virtual void send(const char *buffer, int size, int flags = 0) = 0;
    virtual void send(const uint8_t *buffer, int size, int flags = 0) = 0;
    virtual void send(const std::string &str, int flags = 0) = 0;
    virtual int recv(char *buffer, int size, int flags = 0) = 0;
    virtual int recv(uint8_t *buffer, int size, int flags = 0) = 0;
    virtual int recv(std::vector<char> &buf, int flags = 0) = 0;

    virtual std::string get_host()
    {
        return host;
    }
    virtual int get_port()
    {
        return port;
    }

    typedef std::shared_ptr<cmd::socket> ptr;

protected:
    std::string host = "";
    int port = 0;
};
};

#endif
