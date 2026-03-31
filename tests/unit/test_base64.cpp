#include "io/base64.h"
#include <vector>
#include <string>
#include <iostream>
#include <cassert>

using namespace txtcrypt;

void test_base64_encode_empty() {
    std::vector<uint8_t> empty;
    std::string encoded = base64_encode(empty);
    assert(encoded.empty());
    std::cout << "test_base64_encode_empty passed" << std::endl;
}

void test_base64_encode_single_byte() {
    std::vector<uint8_t> input = { 'M' };
    std::string encoded = base64_encode(input);
    assert(encoded == "TQ==");
    std::cout << "test_base64_encode_single_byte passed" << std::endl;
}

void test_base64_encode_simple() {
    std::vector<uint8_t> input = { 'H', 'e', 'l', 'l', 'o' };
    std::string encoded = base64_encode(input);
    assert(encoded == "SGVsbG8=");
    std::cout << "test_base64_encode_simple passed" << std::endl;
}

void test_base64_decode_roundtrip() {
    std::vector<uint8_t> original = { 'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '!' };
    std::string encoded = base64_encode(original);
    auto decoded = base64_decode(encoded);
    assert(decoded == original);
    std::cout << "test_base64_decode_roundtrip passed" << std::endl;
}

void test_base64_decode_invalid() {
    std::string invalid = "Invalid@Base64!";
    auto decoded = base64_decode(invalid);
    assert(decoded.empty());
    std::cout << "test_base64_decode_invalid passed" << std::endl;
}

void test_base64_encode_two_bytes() {
    std::vector<uint8_t> input = { 'M', 'n' };
    std::string encoded = base64_encode(input);
    assert(encoded == "TW4=");
    std::cout << "test_base64_encode_two_bytes passed" << std::endl;
}

void test_base64_encode_three_bytes() {
    std::vector<uint8_t> input = { 'M', 'a', 'n' };
    std::string encoded = base64_encode(input);
    assert(encoded == "TWFu");
    std::cout << "test_base64_encode_three_bytes passed" << std::endl;
}

void test_base64_encode_binary_data() {
    std::vector<uint8_t> input = {0x00, 0xFF, 0x55, 0xAA};
    std::string encoded = base64_encode(input);
    auto decoded = base64_decode(encoded);
    assert(decoded == input);
    std::cout << "test_base64_encode_binary_data passed" << std::endl;
}

void test_base64_decode_with_padding() {
    std::string encoded = "SGVsbG8=";  // "Hello"
    auto decoded = base64_decode(encoded);
    std::vector<uint8_t> expected = {'H', 'e', 'l', 'l', 'o'};
    assert(decoded == expected);
    std::cout << "test_base64_decode_with_padding passed" << std::endl;
}

void test_base64_decode_double_padding() {
    std::string encoded = "TQ==";  // "M"
    auto decoded = base64_decode(encoded);
    std::vector<uint8_t> expected = {'M'};
    assert(decoded == expected);
    std::cout << "test_base64_decode_double_padding passed" << std::endl;
}

int main() {
    test_base64_encode_empty();
    test_base64_encode_single_byte();
    test_base64_encode_two_bytes();
    test_base64_encode_three_bytes();
    test_base64_encode_simple();
    test_base64_encode_binary_data();
    test_base64_decode_roundtrip();
    test_base64_decode_with_padding();
    test_base64_decode_double_padding();
    test_base64_decode_invalid();
    std::cout << "All Base64 tests passed!" << std::endl;
    return 0;
}
