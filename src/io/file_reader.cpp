#include "io/file_reader.h"
#include <stdexcept>

namespace txtcrypt {

FileReader::FileReader(const std::string& path, size_t buffer_size)
    : path_(path)
    , buffer_size_(buffer_size)
    , current_pos_(0) {

    stream_.open(path, std::ios::binary);
    if (!stream_) {
        throw std::runtime_error("Failed to open file for reading: " + path);
    }

    stream_.seekg(0, std::ios::end);
    file_size_ = stream_.tellg();
    stream_.seekg(0, std::ios::beg);
}

FileReader::~FileReader() {
    if (stream_.is_open()) {
        stream_.close();
    }
}

std::optional<std::vector<uint8_t>> FileReader::read_chunk() {
    if (!stream_ || stream_.eof()) {
        return std::nullopt;
    }

    std::vector<uint8_t> buffer(buffer_size_);
    stream_.read(reinterpret_cast<char*>(buffer.data()), buffer_size_);

    size_t bytes_read = stream_.gcount();
    current_pos_ += bytes_read;

    if (bytes_read == 0) {
        return std::nullopt;
    }

    buffer.resize(bytes_read);
    return buffer;
}

std::vector<uint8_t> FileReader::read_all() {
    std::vector<uint8_t> result(file_size_);

    stream_.seekg(0, std::ios::beg);
    stream_.read(reinterpret_cast<char*>(result.data()), file_size_);

    if (!stream_) {
        throw std::runtime_error("Failed to read entire file");
    }

    current_pos_ = file_size_;
    return result;
}

void FileReader::reset() {
    stream_.clear();
    stream_.seekg(0, std::ios::beg);
    current_pos_ = 0;
}

} // namespace txtcrypt
