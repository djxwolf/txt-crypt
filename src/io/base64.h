#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <cstddef>

namespace txtcrypt {

std::string base64_encode(const std::vector<uint8_t>& data);
std::string base64_encode(const uint8_t* data, size_t len);
std::vector<uint8_t> base64_decode(const std::string& encoded);

} // namespace txtcrypt
