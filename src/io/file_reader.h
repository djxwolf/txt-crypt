#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <fstream>
#include <optional>

namespace txtcrypt {

class FileReader {
public:
    static constexpr size_t DEFAULT_BUFFER_SIZE = 256 * 1024; // 256 KB

    explicit FileReader(const std::string& path, size_t buffer_size = DEFAULT_BUFFER_SIZE);
    ~FileReader();

    bool is_open() const { return stream_.is_open(); }
    uint64_t size() const { return file_size_; }
    uint64_t tell() const { return current_pos_; }

    std::optional<std::vector<uint8_t>> read_chunk();
    std::vector<uint8_t> read_all();
    void reset();

    const std::string& path() const { return path_; }

private:
    std::string path_;
    std::ifstream stream_;
    size_t buffer_size_;
    uint64_t file_size_;
    uint64_t current_pos_;
};

} // namespace txtcrypt
