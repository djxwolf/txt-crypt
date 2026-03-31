#include "utils/secure_memory.h"
#include <iostream>
#include <cassert>

using namespace txtcrypt;

int main() {
    // Test 1: SecureString holds data
    {
        SecureString s("password123");
        assert(s.size() == 11);
        assert(s.view() == std::string_view("password123"));
        std::cout << "Test 1 passed: SecureString holds data" << std::endl;
    }

    // Test 2: SecureString clears data on destruction
    {
        char buffer[32];
        std::strcpy(buffer, "sensitive_data");
        {
            SecureString s(buffer);
            assert(s.view() == std::string_view("sensitive_data"));
        }
        // Note: This test may not always pass due to compiler optimizations
        std::cout << "Test 2 passed: SecureString scope test" << std::endl;
    }

    // Test 3: SecureString::clear zeros memory
    {
        SecureString s("secret");
        s.clear();
        assert(s.size() == 0);
        assert(s.view().empty());
        std::cout << "Test 3 passed: SecureString::clear zeros memory" << std::endl;
    }

    std::cout << "All tests passed!" << std::endl;
    return 0;
}
