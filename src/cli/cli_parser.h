#pragma once

#include <string>
#include <vector>
#include <memory>
#include <stdexcept>

namespace txtcrypt {

enum class Command {
    ENCRYPT,
    DECRYPT
};

struct Options {
    Command command = Command::ENCRYPT;
    std::vector<std::string> input_files;
    std::string* output_path = nullptr;
    bool in_place = false;
    bool no_progress = false;
};

Options parse_cli(int argc, char* argv[]);

} // namespace txtcrypt
