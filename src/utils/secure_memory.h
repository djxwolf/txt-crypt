#pragma once

#include <string>
#include <string_view>
#include <cstring>
#include <memory>

namespace txtcrypt {

class SecureString {
public:
    SecureString() = default;

    explicit SecureString(const char* str) {
        if (str) {
            size_t len = std::strlen(str);
            data_.reset(new char[len + 1]);
            std::memcpy(data_.get(), str, len + 1);
            size_ = len;
        }
    }

    explicit SecureString(std::string_view sv) {
        data_.reset(new char[sv.size() + 1]);
        std::memcpy(data_.get(), sv.data(), sv.size());
        data_[sv.size()] = '\0';
        size_ = sv.size();
    }

    ~SecureString() {
        clear();
    }

    SecureString(const SecureString&) = delete;
    SecureString& operator=(const SecureString&) = delete;

    SecureString(SecureString&& other) noexcept {
        data_ = std::move(other.data_);
        size_ = other.size_;
        other.size_ = 0;
    }

    SecureString& operator=(SecureString&& other) noexcept {
        if (this != &other) {
            clear();
            data_ = std::move(other.data_);
            size_ = other.size_;
            other.size_ = 0;
        }
        return *this;
    }

    std::string_view view() const {
        return size_ > 0 ? std::string_view(data_.get(), size_) : std::string_view();
    }

    size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }

    void clear() {
        if (data_) {
            std::memset(data_.get(), 0, size_);
            data_.reset();
            size_ = 0;
        }
    }

    const char* c_str() const {
        return data_ ? data_.get() : "";
    }

private:
    std::unique_ptr<char[]> data_;
    size_t size_ = 0;
};

} // namespace txtcrypt
