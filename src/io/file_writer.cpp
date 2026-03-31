#include "io/file_writer.h"
#include <stdexcept>

namespace txtcrypt {

FileWriter::FileWriter(const std::string& path, size_t buffer_size)
    : path_(path)
    , buffer_size_(buffer_size)
    , bytes_written_(0) {

    stream_.open(path, std::ios::binary);
    if (!stream_) {
        throw std::runtime_error("Failed to open file for writing: " + path);
    }
}

FileWriter::~FileWriter() {
    if (stream_.is_open()) {
        flush();
        stream_.close();
    }
}

void FileWriter::write_chunk(const std::vector<uint8_t>& data) {
    write(data.data(), data.size());
}

void FileWriter::write(const uint8_t* data, size_t size) {
    if (!stream_) {
        throw std::runtime_error("File not open for writing");
    }

    stream_.write(reinterpret_cast<const char*>(data), size);
    if (!stream_) {
        throw std::runtime_error("Failed to write to file");
    }

    bytes_written_ += size;
}

void FileWriter::write_string(const std::string& str) {
    write(reinterpret_cast<const uint8_t*>(str.data()), str.size());
}

void FileWriter::flush() {
    if (stream_) {
        stream_.flush();
    }
}

} // namespace txtcrypt
