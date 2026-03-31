#include "cli/progress.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <chrono>

namespace txtcrypt {

ProgressBar::ProgressBar(uint64_t total, size_t width)
    : total_(total)
    , width_(width) {
}

void ProgressBar::set_label(const std::string& label) {
    label_ = label;
}

std::string ProgressBar::format_bytes(uint64_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unit_index = 0;
    double size = static_cast<double>(bytes);

    while (size >= 1024.0 && unit_index < 4) {
        size /= 1024.0;
        unit_index++;
    }

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(2) << size << " " << units[unit_index];
    return ss.str();
}

void ProgressBar::draw(uint64_t current) {
    if (finished_) return;

    double percent = total_ > 0 ? static_cast<double>(current) / total_ : 0;
    size_t filled = static_cast<size_t>(std::round(percent * width_));

    std::cout << "\r";
    if (!label_.empty()) {
        std::cout << label_ << " ";
    }

    std::cout << "[";
    for (size_t i = 0; i < width_; ++i) {
        std::cout << (i < filled ? "=" : " ");
    }
    std::cout << "] ";

    std::cout << std::fixed << std::setprecision(1) << (percent * 100.0) << "% ";

    std::cout << format_bytes(current) << "/" << format_bytes(total_);

    static auto start_time = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count();

    if (elapsed > 0 && current > 0) {
        double speed = static_cast<double>(current) / elapsed;
        std::cout << " (" << format_bytes(static_cast<uint64_t>(speed)) << "/s)";
    }

    std::cout << std::flush;
    last_draw_ = current;
}

void ProgressBar::update(uint64_t current) {
    if (total_ > 0) {
        double percent = static_cast<double>(current) / total_;
        double last_percent = static_cast<double>(last_draw_) / total_;

        if (percent - last_percent < 0.01 && current < total_) {
            return;
        }
    }

    draw(current);
}

void ProgressBar::finish() {
    finished_ = true;
    draw(total_);
    std::cout << std::endl;
}

} // namespace txtcrypt
