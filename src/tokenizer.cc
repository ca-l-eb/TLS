#include <string>

#include "tokenizer.h"

char *cmd::tokenizer::get_line(char *buffer, int size, std::string &write_to,
                               cmd::tokenizer::token &tok)
{
    for (int i = 0; i < size; i++) {
        char c = *buffer++;
        if (c == '\n') {
            // Check if last character was \r, if so, remove from write_to
            if (write_to.back() == '\r')
                write_to.pop_back();

            tok = token::COMPLETE;
            return buffer;
        }
        write_to.push_back(c);
    }
    tok = token::WANT_MORE;
    return buffer;
}
