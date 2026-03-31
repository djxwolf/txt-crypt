#include "utils/error_codes.h"
#include <string_view>

namespace txtcrypt {

const char* errorToString(ErrorCode code) {
    switch (code) {
        case ErrorCode::SUCCESS:         return "Success";
        case ErrorCode::IO_ERROR:        return "I/O error";
        case ErrorCode::CRYPTO_ERROR:    return "Cryptography error";
        case ErrorCode::MEMORY_ERROR:    return "Memory allocation error";
        case ErrorCode::INVALID_INPUT:   return "Invalid input";
        case ErrorCode::INVALID_PASSWORD:return "Invalid password";
        case ErrorCode::CORRUPTED_FILE:  return "Corrupted file";
        default:                         return "Unknown error";
    }
}

} // namespace txtcrypt
