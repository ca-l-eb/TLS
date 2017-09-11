#ifndef CMD_HTTP_RESPONSE_H
#define CMD_HTTP_RESPONSE_H

#include <string>
#include <vector>

#include "stream.h"

namespace cmd {

class http_response {
public:
    http_response(cmd::stream &stream);

    int status_code();
    std::string status_message();
    std::string body();
    std::vector<std::string> headers();

private:
    int status;
    std::string body_str, status_message_str;
    std::vector<std::string> headers_list;

    void process_headers();
    void read_response();
};

};

#endif
