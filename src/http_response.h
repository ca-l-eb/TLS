#ifndef CMD_HTTP_RESPONSE_H
#define CMD_HTTP_RESPONSE_H

#include <map>
#include <string>
#include <vector>

#include "stream.h"

namespace cmd
{
class http_response
{
public:
    http_response(cmd::stream &stream);

    int status_code();
    std::string status_message();
    std::string body();
    std::multimap<std::string, std::string> &headers();
    std::string version();

private:
    int status;
    std::string body_str, status_message_str;
    std::string http_version;
    std::vector<std::string> headers_list;
    std::multimap<std::string, std::string> headers_map;
    int length;
    enum body_type { NONE, CHUNKED, LENGTH };
    enum body_type type;

    void read_response(cmd::stream &s);
    void process_headers(cmd::stream &s);
    void check_response_code();
    void add_headers_to_map();
    void check_body_method();
    void check_content_length();
    void check_transfer_encoding();
    void do_chunked(cmd::stream &s);
    void do_content_length(cmd::stream &s);
    void do_read_all(cmd::stream &s);
    void check_headers(cmd::stream &s);
};
}  // namespace cmd

#endif
