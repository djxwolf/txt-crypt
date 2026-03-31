#pragma once

namespace txtcrypt {

enum class ExitCode : int {
    SUCCESS = 0,
    ERROR_FILE_NOT_FOUND = 1,
    ERROR_PERMISSION = 2,
    ERROR_DECRYPTION_FAILED = 3,
    ERROR_DISK_SPACE = 4,
    ERROR_INTEGRITY = 5,
    ERROR_INVALID_ARG = 6,
    ERROR_UNKNOWN = 255
};

enum class ErrorCode : int {
    SUCCESS = 0,
    IO_ERROR,
    CRYPTO_ERROR,
    MEMORY_ERROR,
    INVALID_INPUT,
    INVALID_PASSWORD,
    CORRUPTED_FILE
};

const char* errorToString(ErrorCode code);

} // namespace txtcrypt
