#pragma once

#include <cstdint>
#include <string>
#include <iostream>

namespace txtcrypt {

class ProgressBar {
public:
    explicit ProgressBar(uint64_t total, size_t width = 50);

    void update(uint64_t current);

    void finish();

    void set_label(const std::string& label);

    uint64_t get_total() const { return total_; }

private:
    uint64_t total_;
    size_t width_;
    uint64_t last_draw_ = 0;
    std::string label_;
    bool finished_ = false;

    void draw(uint64_t current);
    std::string format_bytes(uint64_t bytes);
};

} // namespace txtcrypt
