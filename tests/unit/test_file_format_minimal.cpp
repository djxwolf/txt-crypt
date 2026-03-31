#include "core/file_format.h"
#include <iostream>
#include <cassert>
#include <cstring>

using namespace txtcrypt;

int main() {
    // Test 1: File format constants are correct
    {
        const char* expected_magic = "TXTCRYPT";
        bool magic_match = true;
        for (int i = 0; i < 8; i++) {
            if (FILE_MAGIC[i] != expected_magic[i]) {
                magic_match = false;
                break;
            }
        }
        assert(magic_match);
        assert(FILE_FORMAT_VERSION == 1);
        std::cout << "Test 1 passed: File format constants are correct" << std::endl;
    }

    // Test 2: FileHeader size is correct
    {
        assert(sizeof(FileHeader) == 16);
        std::cout << "Test 2 passed: FileHeader size is 16 bytes" << std::endl;
    }

    // Test 3: FileHeader default values
    {
        FileHeader header;
        bool magic_match = true;
        const char* expected_magic = "TXTCRYPT";
        for (int i = 0; i < 8; i++) {
            if (header.magic[i] != expected_magic[i]) {
                magic_match = false;
                break;
            }
        }
        assert(magic_match);
        assert(header.version == FILE_FORMAT_VERSION);
        assert(header.salt_len == SALT_SIZE);
        assert(header.iv_len == IV_SIZE);
        std::cout << "Test 3 passed: FileHeader default values are correct" << std::endl;
    }

    // Test 4: CryptoParams structure size
    {
        CryptoParams params;
        assert(params.salt.size() == SALT_SIZE);
        assert(params.iv.size() == IV_SIZE);
        assert(params.tag.size() == TAG_SIZE);
        std::cout << "Test 4 passed: CryptoParams has correct sizes" << std::endl;
    }

    std::cout << "All file format tests passed!" << std::endl;
    return 0;
}
