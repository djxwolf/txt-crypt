#pragma once

#include <vector>
#include <array>
#include <cstdint>
#include "core/file_format.h"
#include "utils/secure_memory.h"

namespace txtcrypt {

struct EncryptionResult {
    std::vector<uint8_t> ciphertext;
    CryptoParams params;
};

struct ChunkEncryptionResult {
    std::vector<uint8_t> ciphertext;
    std::array<uint8_t, TAG_SIZE> tag;
};

class CryptoEngine {
public:
    explicit CryptoEngine(const SecureString& password);

    EncryptionResult encrypt(const std::vector<uint8_t>& plaintext);
    std::vector<uint8_t> decrypt(
        const std::vector<uint8_t>& ciphertext,
        const CryptoParams& params
    );

    std::vector<uint8_t> encrypt_with_params(
        const std::vector<uint8_t>& plaintext,
        const CryptoParams& params
    );

    std::vector<uint8_t> decrypt_with_key(
        const std::vector<uint8_t>& ciphertext,
        const std::array<uint8_t, KEY_SIZE>& key,
        const std::array<uint8_t, IV_SIZE>& iv,
        const std::array<uint8_t, TAG_SIZE>& tag
    );

    // Streaming encryption methods
    ChunkEncryptionResult encrypt_chunk(
        const std::vector<uint8_t>& chunk,
        const std::array<uint8_t, KEY_SIZE>& key,
        const std::array<uint8_t, IV_SIZE>& base_iv,
        uint64_t chunk_index
    );

    std::vector<uint8_t> decrypt_chunk(
        const std::vector<uint8_t>& ciphertext,
        const std::array<uint8_t, KEY_SIZE>& key,
        const std::array<uint8_t, IV_SIZE>& base_iv,
        uint64_t chunk_index,
        const std::array<uint8_t, TAG_SIZE>& tag
    );

private:
    SecureString password_;
};

} // namespace txtcrypt
