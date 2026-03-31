#include "utils/temp_file.h"
#include <fstream>
#include <random>
#include <sstream>
#include <iomanip>

namespace txtcrypt {

namespace {
std::string generate_temp_suffix() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);

    std::ostringstream ss;
    ss << ".tmp";
    for (int i = 0; i < 8; ++i) {
        ss << std::hex << dis(gen);
    }
    return ss.str();
}
}

TempFile::TempFile(std::string target_path, std::string temp_path, bool is_backup)
    : target_path_(std::move(target_path))
    , temp_path_(std::move(temp_path))
    , is_backup_(is_backup) {
}

TempFile TempFile::create(const std::string& target_path) {
    std::string temp_path = target_path + generate_temp_suffix();
    return TempFile(target_path, temp_path, false);
}

TempFile TempFile::create_backup(const std::string& target_path) {
    std::string backup_path = target_path + ".bak";

    std::ifstream src(target_path, std::ios::binary);
    std::ofstream dst(backup_path, std::ios::binary);

    if (src && dst) {
        dst << src.rdbuf();
    }

    return TempFile(target_path, backup_path, true);
}

void TempFile::commit() {
    if (committed_) {
        return;
    }

    if (!is_backup_) {
        std::filesystem::rename(temp_path_, target_path_);
    }

    committed_ = true;
}

void TempFile::rollback() {
    if (!committed_ && std::filesystem::exists(temp_path_)) {
        std::filesystem::remove(temp_path_);
    }
    committed_ = true;
}

TempFile::~TempFile() {
    rollback();
}

TempFile::TempFile(TempFile&& other) noexcept
    : target_path_(std::move(other.target_path_))
    , temp_path_(std::move(other.temp_path_))
    , is_backup_(other.is_backup_)
    , committed_(other.committed_) {
    other.committed_ = true;
}

TempFile& TempFile::operator=(TempFile&& other) noexcept {
    if (this != &other) {
        rollback();

        target_path_ = std::move(other.target_path_);
        temp_path_ = std::move(other.temp_path_);
        is_backup_ = other.is_backup_;
        committed_ = other.committed_;
        other.committed_ = true;
    }
    return *this;
}

} // namespace txtcrypt
