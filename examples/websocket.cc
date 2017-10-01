#include <string>

#include "socket.h"
#include "websocket.h"

int main(int argc, char *argv[])
{
    std::string resource{"/gateway"};
    cmd::websocket sock{resource, true};
    sock.connect("www.google.com", 80);
}
