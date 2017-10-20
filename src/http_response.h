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
    http_response();
    explicit http_response(cmd::stream &stream);
    int status_code();
    std::string status_message();
    std::string body();
    std::multimap<std::string, std::string> &headers();
    std::string version();

private:
    int status;
    std::string http_version;
    std::string status_message_str;
    std::string body_str;
    std::multimap<std::string, std::string> headers_map;
    size_t length;
    enum class body_type { NONE, CHUNKED, LENGTH, ALL } type;

    void read_response(cmd::stream &s);
    void process_headers(cmd::stream &s);
    void check_response_status(const std::string &status_line);
    void add_header_to_map(const std::string &line);
    void check_body_method();
    void check_content_length();
    void check_transfer_encoding();
    void do_chunked(cmd::stream &s);
    void do_content_length(cmd::stream &s);
    void do_read_all(cmd::stream &s);
    void check_connection_close(cmd::stream &s);
};
}  // namespace cmd

#endif
