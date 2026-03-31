#pragma once

#include <cstdint>
#include <array>
#include <cstring>

namespace txtcrypt {

constexpr size_t SALT_SIZE = 16;
constexpr size_t IV_SIZE = 12;
constexpr size_t TAG_SIZE = 16;
constexpr size_t KEY_SIZE = 32;
constexpr size_t HEADER_SIZE = 16;

constexpr uint8_t FILE_FORMAT_VERSION = 1;

constexpr char FILE_MAGIC[8] = { 'T', 'X', 'T', 'C', 'R', 'Y', 'P', 'T' };

#pragma pack(push, 1)
struct FileHeader {
    char magic[8];       // "TXTCRYPT"
    uint8_t version;     // Format version
    uint8_t salt_len;    // Salt length
    uint8_t iv_len;      // IV length
    uint8_t reserved[5]; // Reserved for future use

    FileHeader() {
        std::memcpy(magic, FILE_MAGIC, 8);
        version = FILE_FORMAT_VERSION;
        salt_len = SALT_SIZE;
        iv_len = IV_SIZE;
        std::memset(reserved, 0, 5);
    }
};
#pragma pack(pop)

static_assert(sizeof(FileHeader) == HEADER_SIZE, "FileHeader must be 16 bytes");

struct CryptoParams {
    std::array<uint8_t, SALT_SIZE> salt;
    std::array<uint8_t, IV_SIZE> iv;
    std::array<uint8_t, TAG_SIZE> tag;
};

} // namespace txtcrypt
