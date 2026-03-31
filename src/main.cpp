#include "cli/cli_parser.h"
#include "cli/progress.h"
#include "core/stream_processor.h"
#include "utils/error_codes.h"
#include "utils/secure_memory.h"
#include <iostream>
#include <termios.h>
#include <unistd.h>

using namespace txtcrypt;

namespace {
std::string read_password() {
    std::cout << "Enter password: ";
    std::cout.flush();

    termios old_t, new_t;
    tcgetattr(STDIN_FILENO, &old_t);
    new_t = old_t;
    new_t.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_t);

    std::string password;
    std::getline(std::cin, password);

    tcsetattr(STDIN_FILENO, TCSANOW, &old_t);
    std::cout << std::endl;

    return password;
}
} // namespace

int main(int argc, char* argv[]) {
    try {
        auto options = parse_cli(argc, argv);

        if (options.input_files.empty()) {
            std::cerr << "Error: No input files specified" << std::endl;
            return static_cast<int>(ExitCode::ERROR_INVALID_ARG);
        }

        std::string pwd_str = read_password();
        SecureString password(pwd_str);
        std::fill(pwd_str.begin(), pwd_str.end(), '\0');

        StreamProcessor processor(password);

        auto make_progress = [&options](const std::string& label, uint64_t size) {
            if (options.no_progress) {
                return std::unique_ptr<ProgressBar>();
            }
            auto bar = std::make_unique<ProgressBar>(size);
            bar->set_label(label);
            return bar;
        };

        if (options.in_place) {
            for (const auto& input : options.input_files) {
                auto progress = make_progress("Processing", 0);

                auto callback = [&progress](uint64_t processed, uint64_t total) {
                    if (progress) {
                        if (progress->get_total() != total) {
                            progress = std::make_unique<ProgressBar>(total);
                        }
                        progress->update(processed);
                    }
                };

                if (options.command == Command::ENCRYPT) {
                    processor.encrypt_file_in_place(input, callback);
                } else {
                    processor.decrypt_file_in_place(input, callback);
                }

                if (progress) progress->finish();
            }
        } else if (options.output_path) {
            for (const auto& input : options.input_files) {
                auto progress = make_progress("Processing", 0);

                auto callback = [&progress](uint64_t processed, uint64_t total) {
                    if (progress) {
                        if (progress->get_total() != total) {
                            progress = std::make_unique<ProgressBar>(total);
                        }
                        progress->update(processed);
                    }
                };

                if (options.command == Command::ENCRYPT) {
                    processor.encrypt_file(input, *options.output_path, callback);
                } else {
                    processor.decrypt_file(input, *options.output_path, callback);
                }

                if (progress) progress->finish();
            }
        } else {
            std::cerr << "Error: Must specify --output or --in-place" << std::endl;
            return static_cast<int>(ExitCode::ERROR_INVALID_ARG);
        }

        return static_cast<int>(ExitCode::SUCCESS);

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return static_cast<int>(ExitCode::ERROR_UNKNOWN);
    }
}
