#pragma once

#include <string>
#include <filesystem>

namespace txtcrypt {

class TempFile {
public:
    static TempFile create(const std::string& target_path);
    static TempFile create_backup(const std::string& target_path);

    std::string path() const { return temp_path_; }

    void commit();
    void rollback();

    ~TempFile();

    TempFile(const TempFile&) = delete;
    TempFile& operator=(const TempFile&) = delete;
    TempFile(TempFile&& other) noexcept;
    TempFile& operator=(TempFile&& other) noexcept;

private:
    TempFile(std::string target_path, std::string temp_path, bool is_backup);

    std::string target_path_;
    std::string temp_path_;
    bool is_backup_;
    bool committed_ = false;
};

} // namespace txtcrypt
