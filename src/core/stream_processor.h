#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <functional>
#include "core/crypto_engine.h"
#include "utils/secure_memory.h"

namespace txtcrypt {

using ProgressCallback = std::function<void(uint64_t bytes_processed, uint64_t total_bytes)>;

class StreamProcessor {
public:
    static constexpr size_t DEFAULT_BUFFER_SIZE = 1024 * 1024;

    StreamProcessor(const SecureString& password, size_t buffer_size = DEFAULT_BUFFER_SIZE);

    void encrypt_file(
        const std::string& input_path,
        const std::string& output_path,
        ProgressCallback progress = nullptr
    );

    void decrypt_file(
        const std::string& input_path,
        const std::string& output_path,
        ProgressCallback progress = nullptr
    );

    void encrypt_file_in_place(
        const std::string& file_path,
        ProgressCallback progress = nullptr
    );

    void decrypt_file_in_place(
        const std::string& file_path,
        ProgressCallback progress = nullptr
    );

    void encrypt_files(
        const std::vector<std::string>& input_paths,
        const std::string& output_dir,
        ProgressCallback progress = nullptr
    );

    CryptoEngine& engine() { return engine_; }

private:
    SecureString password_;
    size_t buffer_size_;
    CryptoEngine engine_;

    void write_header(const std::string& path, const CryptoParams& params);
    CryptoParams read_header(const std::string& path);
};

} // namespace txtcrypt
