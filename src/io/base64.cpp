#include "io/base64.h"
#include <stdexcept>

namespace txtcrypt {

namespace {
constexpr const char* alphabet =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

inline bool is_base64_char(char c) {
    return (c >= 'A' && c <= 'Z') ||
           (c >= 'a' && c <= 'z') ||
           (c >= '0' && c <= '9') ||
           c == '+' || c == '/' || c == '=';
}
}

std::string base64_encode(const std::vector<uint8_t>& data) {
    return base64_encode(data.data(), data.size());
}

std::string base64_encode(const uint8_t* data, size_t len) {
    if (!data || len == 0) {
        return "";
    }

    std::string result;
    result.reserve(((len + 2) / 3) * 4);

    for (size_t i = 0; i < len; i += 3) {
        uint32_t triple = (static_cast<uint32_t>(data[i]) << 16);

        if (i + 1 < len) {
            triple |= (static_cast<uint32_t>(data[i + 1]) << 8);
        }
        if (i + 2 < len) {
            triple |= data[i + 2];
        }

        result.push_back(alphabet[(triple >> 18) & 0x3F]);
        result.push_back(alphabet[(triple >> 12) & 0x3F]);

        if (i + 1 < len) {
            result.push_back(alphabet[(triple >> 6) & 0x3F]);
        } else {
            result.push_back('=');
        }

        if (i + 2 < len) {
            result.push_back(alphabet[triple & 0x3F]);
        } else {
            result.push_back('=');
        }
    }

    return result;
}

std::vector<uint8_t> base64_decode(const std::string& encoded) {
    if (encoded.empty()) {
        return {};
    }

    for (char c : encoded) {
        if (!is_base64_char(c)) {
            return {};
        }
    }

    uint8_t lookup[256] = {0};
    for (int i = 0; i < 64; ++i) {
        lookup[static_cast<uint8_t>(alphabet[i])] = static_cast<uint8_t>(i);
    }

    std::vector<uint8_t> result;
    result.reserve((encoded.size() / 4) * 3);

    for (size_t i = 0; i < encoded.size(); i += 4) {
        uint32_t quad = 0;

        for (size_t j = 0; j < 4; ++j) {
            if (i + j >= encoded.size()) break;

            char c = encoded[i + j];
            if (c == '=') {
                quad <<= 6;
            } else {
                quad = (quad << 6) | lookup[static_cast<uint8_t>(c)];
            }
        }

        result.push_back(static_cast<uint8_t>((quad >> 16) & 0xFF));

        if (i + 2 < encoded.size() && encoded[i + 2] != '=') {
            result.push_back(static_cast<uint8_t>((quad >> 8) & 0xFF));
        }

        if (i + 3 < encoded.size() && encoded[i + 3] != '=') {
            result.push_back(static_cast<uint8_t>(quad & 0xFF));
        }
    }

    return result;
}

} // namespace txtcrypt
