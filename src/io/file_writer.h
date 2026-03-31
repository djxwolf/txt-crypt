#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <fstream>

namespace txtcrypt {

class FileWriter {
public:
    static constexpr size_t DEFAULT_BUFFER_SIZE = 256 * 1024; // 256 KB

    explicit FileWriter(const std::string& path, size_t buffer_size = DEFAULT_BUFFER_SIZE);
    ~FileWriter();

    bool is_open() const { return stream_.is_open(); }

    void write_chunk(const std::vector<uint8_t>& data);
    void write(const uint8_t* data, size_t size);
    void write_string(const std::string& str);
    void flush();

    uint64_t bytes_written() const { return bytes_written_; }
    const std::string& path() const { return path_; }

private:
    std::string path_;
    std::ofstream stream_;
    size_t buffer_size_;
    uint64_t bytes_written_;
};

} // namespace txtcrypt
