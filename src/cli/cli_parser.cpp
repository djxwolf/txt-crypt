#include "cli/cli_parser.h"
#include "cli/progress.h"
#include "core/stream_processor.h"
#include "utils/error_codes.h"
#include "utils/secure_memory.h"
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <getopt.h>

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

namespace txtcrypt {

Options parse_cli(int argc, char* argv[]) {
    Options options;
    
    static struct option long_options[] = {
        {"encrypt", no_argument, 0, 'e'},
        {"decrypt", no_argument, 0, 'd'},
        {"output", required_argument, 0, 'o'},
        {"in-place", no_argument, 0, 'i'},
        {"no-progress", no_argument, 0, 'q'},
        {0, 0, 0, 0}
    };

    int c;
    while ((c = getopt_long(argc, argv, "edo:iq", long_options, nullptr)) != -1) {
        switch (c) {
            case 'e':
                options.command = Command::ENCRYPT;
                break;
            case 'd':
                options.command = Command::DECRYPT;
                break;
            case 'o':
                options.output_path = new std::string(optarg);
                break;
            case 'i':
                options.in_place = true;
                break;
            case 'q':
                options.no_progress = true;
                break;
            default:
                throw std::runtime_error("Invalid arguments");
        }
    }

    for (int i = optind; i < argc; ++i) {
        options.input_files.push_back(argv[i]);
    }

    return options;
}

} // namespace txtcrypt

