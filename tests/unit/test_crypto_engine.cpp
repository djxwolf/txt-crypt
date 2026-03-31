#include "core/crypto_engine.h"
#include "core/kdf.h"
#include "utils/secure_memory.h"
#include "core/file_format.h"
#include <cassert>
#include <iostream>

using namespace txtcrypt;

void test_crypto_encrypt_decrypt() {
    SecureString password("test_password");
    CryptoEngine engine(password);

    std::vector<uint8_t> plaintext = { 'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd' };

    auto result = engine.encrypt(plaintext);

    assert(!result.ciphertext.empty());
    std::array<uint8_t, SALT_SIZE> zero_salt{};
    std::array<uint8_t, IV_SIZE> zero_iv{};
    std::array<uint8_t, TAG_SIZE> zero_tag{};
    assert(result.params.salt != zero_salt);
    assert(result.params.iv != zero_iv);
    assert(result.params.tag != zero_tag);

    std::vector<uint8_t> decrypted;
    decrypted = engine.decrypt(result.ciphertext, result.params);
    assert(decrypted == plaintext);
    printf("test_crypto_encrypt_decrypt passed\n");
}

void test_crypto_different_iv() {
    SecureString password("test_password");
    CryptoEngine engine(password);

    std::vector<uint8_t> plaintext = { 'T', 'e', 's', 't' };

    auto result1 = engine.encrypt(plaintext);
    auto result2 = engine.encrypt(plaintext);

    assert(result1.ciphertext != result2.ciphertext);
    assert(result1.params.iv != result2.params.iv);
    printf("test_crypto_different_iv passed\n");
}

void test_crypto_wrong_password() {
    SecureString password1("password1");
    SecureString password2("password2");

    CryptoEngine engine1(password1);
    CryptoEngine engine2(password2);

    std::vector<uint8_t> plaintext = { 'S', 'e', 'c', 'r', 'e', 't' };
    auto encrypted = engine1.encrypt(plaintext);

    bool threw = false;
    try {
        engine2.decrypt(encrypted.ciphertext, encrypted.params);
    } catch (...) {
        threw = true;
    }
    assert(threw);
    printf("test_crypto_wrong_password passed\n");
}

void test_crypto_empty_plaintext() {
    SecureString password("test_password");
    CryptoEngine engine(password);

    std::vector<uint8_t> empty;
    auto result = engine.encrypt(empty);

    assert(!result.ciphertext.empty());
    printf("test_crypto_empty_plaintext passed\n");
}

void test_crypto_modified_tag_fails() {
    SecureString password("test_password");
    CryptoEngine engine(password);

    std::vector<uint8_t> plaintext = { 'A', 'u', 't', 'h' };
    auto encrypted = engine.encrypt(plaintext);

    encrypted.params.tag[0] ^= 0xFF;

    bool threw = false;
    try {
        engine.decrypt(encrypted.ciphertext, encrypted.params);
    } catch (...) {
        threw = true;
    }
    assert(threw);
    printf("test_crypto_modified_tag_fails passed\n");
}

int main() {
    test_crypto_encrypt_decrypt();
    test_crypto_different_iv();
    test_crypto_wrong_password();
    test_crypto_empty_plaintext();
    test_crypto_modified_tag_fails();
    printf("All CryptoEngine tests passed!\n");
    return 0;
}
