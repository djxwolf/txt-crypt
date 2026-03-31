#include "utils/secure_memory.h"
#include "core/kdf.h"
#include "core/file_format.h"
#include <iostream>
#include <cassert>
#include <array>

using namespace txtcrypt;

void test_kdf_derives_key() {
    SecureString password("test_password");
    std::array<uint8_t, SALT_SIZE> salt = {};

    auto key = derive_key(password, salt);

    assert(key.size() == KEY_SIZE);
    assert(key[0] != 0);
    std::cout << "test_kdf_derives_key passed" << std::endl;
}

void test_kdf_deterministic() {
    SecureString password("same_password");
    std::array<uint8_t, SALT_SIZE> salt = {};

    auto key1 = derive_key(password, salt);
    auto key2 = derive_key(password, salt);

    assert(key1 == key2);
    std::cout << "test_kdf_deterministic passed" << std::endl;
}

void test_kdf_different_salt() {
    SecureString password("same_password");
    std::array<uint8_t, SALT_SIZE> salt1 = {};
    std::array<uint8_t, SALT_SIZE> salt2 = {};
    salt2[0] = 1;

    auto key1 = derive_key(password, salt1);
    auto key2 = derive_key(password, salt2);

    assert(key1 != key2);
    std::cout << "test_kdf_different_salt passed" << std::endl;
}

void test_kdf_random_salt() {
    auto salt = generate_salt();

    bool all_zeros = true;
    for (auto byte : salt) {
        if (byte != 0) {
            all_zeros = false;
            break;
        }
    }
    assert(!all_zeros);
    std::cout << "test_kdf_random_salt passed" << std::endl;
}

void test_kdf_unique_salts() {
    auto salt1 = generate_salt();
    auto salt2 = generate_salt();

    assert(salt1 != salt2);
    std::cout << "test_kdf_unique_salts passed" << std::endl;
}

int main() {
    test_kdf_derives_key();
    test_kdf_deterministic();
    test_kdf_different_salt();
    test_kdf_random_salt();
    test_kdf_unique_salts();
    std::cout << "All KDF tests passed!" << std::endl;
    return 0;
}
