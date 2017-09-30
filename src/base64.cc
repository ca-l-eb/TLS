#include <cmath>
#include <string>
#include <vector>

#include "base64.h"

static const char b64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const unsigned char inv_table[256] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  62, 0,  0,  0,  63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  3,  4,  5,  6,
    7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0,  0,  0,  0,  0,
    0,  26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
    49, 50, 51, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0};

std::string cmd::base64::encode(const std::string &message)
{
    return encode(message.c_str(), message.length());
}

std::string cmd::base64::encode(const char *message, size_t size)
{
    int out_size = static_cast<int>(std::ceil(size / 3.0f)) * 4;
    std::string encoded;
    encoded.resize(out_size);
    int loc = 0;
    while (size > 2) {
        // Process the next 3 bytes
        encoded[loc++] = b64_table[(message[0] >> 2)];
        encoded[loc++] = b64_table[(message[0] & 0x03) << 4 | message[1] >> 4];
        encoded[loc++] = b64_table[(message[1] & 0x0F) << 2 | message[2] >> 6];
        encoded[loc++] = b64_table[(message[2] & 0x3F)];
        message += 3;
        size -= 3;
    }
    if (size == 1) {
        // Clear last 2 bytes because they dont really exist in the message
        // Only message 2 of the Base64 encoded bytes
        encoded[loc++] = b64_table[(message[0] >> 2)];
        encoded[loc++] = b64_table[(message[0] & 0x03) << 4 | message[1] >> 4];
        encoded[loc++] = '=';
        encoded[loc] = '=';
    } else if (size == 2) {
        // Only message 3 of the Base64 encoded bytes
        encoded[loc++] = b64_table[(message[0] >> 2)];
        encoded[loc++] = b64_table[(message[0] & 0x03) << 4 | message[1] >> 4];
        encoded[loc++] = b64_table[(message[1] & 0x0F) << 2 | message[2] >> 6];
        encoded[loc] = '=';
    }
    return encoded;
}

std::vector<unsigned char> cmd::base64::decode(const std::string &message)
{
    return decode(message.c_str(), message.length());
}

std::vector<unsigned char> cmd::base64::decode(const char *message, size_t size)
{
    if (message[size - 1] == '=') {
        size--;
        if (message[size - 2] == '=')
            size--;
    }
    int out_size = (3 * size) / 4;
    std::vector<unsigned char> decoded(out_size);
    int loc = 0;
    while (size > 3) {
        decoded[loc++] = inv_table[message[0]] << 2 | inv_table[message[1]] >> 4;
        decoded[loc++] = inv_table[message[1]] << 4 | inv_table[message[2]] >> 2;
        decoded[loc++] = inv_table[message[2]] << 6 | inv_table[message[3]];
        message += 4;
        size -= 4;
    }
    if (size == 3) {
        // There are 2 more data bytes, 3 more Base64 characters
        decoded[loc++] = inv_table[message[0]] << 2 | inv_table[message[1]] >> 4;
        decoded[loc] = inv_table[message[1]] << 4 | inv_table[message[2]] >> 2;
    } else if (size == 2) {
        // There is 1 more data byte, 2 Base64 characters
        decoded[loc] = inv_table[message[0]] << 2 | inv_table[message[1]] >> 4;
    }
    return decoded;
}
