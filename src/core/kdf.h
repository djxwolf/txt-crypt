#pragma once

#include <array>
#include <vector>
#include <cstdint>
#include "core/file_format.h"
#include "utils/secure_memory.h"

namespace txtcrypt {

struct KdfParams {
    uint32_t t_cost = 3;
    uint32_t m_cost = 65536;
    uint32_t parallelism = 4;
};

std::array<uint8_t, KEY_SIZE> derive_key(
    const SecureString& password,
    const std::array<uint8_t, SALT_SIZE>& salt,
    const KdfParams& params = {}
);

std::array<uint8_t, SALT_SIZE> generate_salt();
std::vector<uint8_t> random_bytes(size_t size);

} // namespace txtcrypt
