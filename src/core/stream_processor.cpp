#include "core/stream_processor.h"
#include "io/file_reader.h"
#include "io/file_writer.h"
#include "io/base64.h"
#include "utils/temp_file.h"
#include "core/kdf.h"
#include <stdexcept>
#include <filesystem>
#include <cstring>

namespace txtcrypt {

StreamProcessor::StreamProcessor(const SecureString& password, size_t buffer_size)
    : password_(password.view())
    , buffer_size_(buffer_size)
    , engine_(password) {
}

void StreamProcessor::write_header(const std::string& path, const CryptoParams& params) {
    FileWriter writer(path, 1024);

    FileHeader header;
    std::memcpy(header.magic, FILE_MAGIC, 8);
    header.version = FILE_FORMAT_VERSION;
    header.salt_len = SALT_SIZE;
    header.iv_len = IV_SIZE;
    std::memset(header.reserved, 0, 5);

    writer.write(reinterpret_cast<const uint8_t*>(&header), sizeof(header));
    writer.write(params.salt.data(), params.salt.size());
    writer.write(params.iv.data(), params.iv.size());
    writer.write(params.tag.data(), params.tag.size());

    writer.flush();
}

CryptoParams StreamProcessor::read_header(const std::string& path) {
    FileReader reader(path, 1024);

    FileHeader header;
    auto data = reader.read_all();

    if (data.size() < sizeof(FileHeader) + SALT_SIZE + IV_SIZE) {
        throw std::runtime_error("File too small to be encrypted");
    }

    std::memcpy(&header, data.data(), sizeof(FileHeader));

    if (std::string(header.magic, 8) != std::string(FILE_MAGIC, 8)) {
        throw std::runtime_error("Invalid file format (missing magic)");
    }

    if (header.version != FILE_FORMAT_VERSION) {
        throw std::runtime_error("Unsupported file version");
    }

    CryptoParams params;

    size_t offset = sizeof(FileHeader);
    std::memcpy(params.salt.data(), data.data() + offset, SALT_SIZE);
    offset += SALT_SIZE;
    std::memcpy(params.iv.data(), data.data() + offset, IV_SIZE);
    // Note: In streaming format, tags are embedded with data
    // This function is only used for in-place operations

    return params;
}

void StreamProcessor::encrypt_file(
    const std::string& input_path,
    const std::string& output_path,
    ProgressCallback progress
) {
    // Generate salt and base IV
    auto salt = generate_salt();
    auto base_iv = random_bytes(IV_SIZE);
    std::array<uint8_t, IV_SIZE> base_iv_arr;
    std::copy(base_iv.begin(), base_iv.end(), base_iv_arr.begin());

    // Derive key once
    auto key = derive_key(password_, salt);

    FileReader reader(input_path, buffer_size_);
    FileWriter writer(output_path, buffer_size_);

    // Write header
    FileHeader header;
    std::memcpy(header.magic, FILE_MAGIC, 8);
    header.version = FILE_FORMAT_VERSION;
    header.salt_len = SALT_SIZE;
    header.iv_len = IV_SIZE;
    std::memset(header.reserved, 0, 5);

    writer.write(reinterpret_cast<const uint8_t*>(&header), sizeof(header));
    writer.write(salt.data(), SALT_SIZE);
    writer.write(base_iv_arr.data(), IV_SIZE);

    // Stream encryption: process chunks
    uint64_t total_size = reader.size();
    uint64_t processed = 0;
    uint64_t chunk_index = 0;

    while (auto chunk = reader.read_chunk()) {
        // Encrypt this chunk with unique IV
        auto result = engine_.encrypt_chunk(*chunk, key, base_iv_arr, chunk_index);

        // Write tag followed by ciphertext
        writer.write(result.tag.data(), TAG_SIZE);
        writer.write_chunk(result.ciphertext);

        processed += chunk->size();
        chunk_index++;

        if (progress) {
            progress(processed, total_size);
        }
    }

    if (progress) {
        progress(total_size, total_size);
    }
}

void StreamProcessor::decrypt_file(
    const std::string& input_path,
    const std::string& output_path,
    ProgressCallback progress
) {
    // Read header to get salt and base IV
    FileReader header_reader(input_path, 4096);
    auto header_data = header_reader.read_all();

    if (header_data.size() < sizeof(FileHeader) + SALT_SIZE + IV_SIZE) {
        throw std::runtime_error("File too small to be encrypted");
    }

    FileHeader header;
    std::memcpy(&header, header_data.data(), sizeof(FileHeader));

    if (std::string(header.magic, 8) != std::string(FILE_MAGIC, 8)) {
        throw std::runtime_error("Invalid file format (missing magic)");
    }

    if (header.version != FILE_FORMAT_VERSION) {
        throw std::runtime_error("Unsupported file version");
    }

    // Extract salt and base IV
    std::array<uint8_t, SALT_SIZE> salt;
    std::array<uint8_t, IV_SIZE> base_iv;

    size_t offset = sizeof(FileHeader);
    std::memcpy(salt.data(), header_data.data() + offset, SALT_SIZE);
    offset += SALT_SIZE;
    std::memcpy(base_iv.data(), header_data.data() + offset, IV_SIZE);
    offset += IV_SIZE;

    // Derive key
    auto key = derive_key(password_, salt);

    // Calculate total file size
    uint64_t total_file_size = 0;
    {
        FileReader size_reader(input_path, buffer_size_);
        total_file_size = size_reader.size();
    }

    size_t header_total_size = sizeof(FileHeader) + SALT_SIZE + IV_SIZE;
    uint64_t total_encrypted_size = total_file_size - header_total_size;

    // Stream decryption: process chunks
    FileReader reader(input_path, buffer_size_);
    FileWriter writer(output_path, buffer_size_);

    uint64_t processed = 0;
    uint64_t chunk_index = 0;

    // Buffer to hold data (tag + ciphertext)
    std::vector<uint8_t> data_buffer;
    data_buffer.reserve(buffer_size_ * 2);

    // Read all data and skip header
    reader.reset();
    bool header_skipped = false;

    while (auto chunk = reader.read_chunk()) {
        if (!header_skipped) {
            // Skip header from first chunk
            if (chunk->size() < header_total_size) {
                throw std::runtime_error("Invalid encrypted file");
            }
            data_buffer.insert(data_buffer.end(), chunk->begin() + header_total_size, chunk->end());
            header_skipped = true;
        } else {
            data_buffer.insert(data_buffer.end(), chunk->begin(), chunk->end());
        }
    }

    // Now process the buffer: tag + ciphertext for each chunk
    size_t pos = 0;
    while (pos < data_buffer.size()) {
        // Check if we have enough data for tag
        if (pos + TAG_SIZE > data_buffer.size()) {
            throw std::runtime_error("Incomplete tag at end of file");
        }

        // Extract tag
        std::array<uint8_t, TAG_SIZE> tag;
        std::memcpy(tag.data(), data_buffer.data() + pos, TAG_SIZE);
        pos += TAG_SIZE;

        // Calculate remaining data size
        size_t remaining = data_buffer.size() - pos;

        // Determine chunk size
        // During encryption, plaintext chunks were buffer_size_ (except last)
        // Ciphertext size equals plaintext size in GCM
        size_t chunk_size = std::min(buffer_size_, remaining);

        if (chunk_size == 0) break;

        // Extract ciphertext
        std::vector<uint8_t> ciphertext(data_buffer.begin() + pos, data_buffer.begin() + pos + chunk_size);
        pos += chunk_size;

        // Decrypt this chunk
        try {
            auto plaintext = engine_.decrypt_chunk(ciphertext, key, base_iv, chunk_index, tag);
            writer.write_chunk(plaintext);
            processed += plaintext.size();
        } catch (const std::exception& e) {
            throw std::runtime_error("Decryption failed - wrong password or corrupted file");
        }

        chunk_index++;

        if (progress) {
            progress(processed, total_encrypted_size);
        }
    }

    if (progress) {
        progress(processed, total_encrypted_size);
    }
}

void StreamProcessor::encrypt_file_in_place(
    const std::string& file_path,
    ProgressCallback progress
) {
    std::string temp_path = file_path + ".enc.tmp";

    encrypt_file(file_path, temp_path, progress);

    std::filesystem::remove(file_path);
    std::filesystem::rename(temp_path, file_path);
}

void StreamProcessor::decrypt_file_in_place(
    const std::string& file_path,
    ProgressCallback progress
) {
    std::string temp_path = file_path + ".dec.tmp";

    decrypt_file(file_path, temp_path, progress);

    std::filesystem::remove(file_path);
    std::filesystem::rename(temp_path, file_path);
}

void StreamProcessor::encrypt_files(
    const std::vector<std::string>& input_paths,
    const std::string& output_dir,
    ProgressCallback progress
) {
    for (const auto& input_path : input_paths) {
        std::filesystem::path input(input_path);
        std::filesystem::path output = std::filesystem::path(output_dir) / input.filename();

        output += ".enc";

        encrypt_file(input_path, output.string(), progress);
    }
}

} // namespace txtcrypt
