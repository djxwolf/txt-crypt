#define MBEDTLS_DECLARE_PRIVATE_IDENTIFIERS
#include "core/crypto_engine.h"
#include "core/kdf.h"
#include <stdexcept>
#include <cstring>

#include "mbedtls/private/gcm.h"
#include "mbedtls/error.h"

namespace txtcrypt {

CryptoEngine::CryptoEngine(SecureString password)
    : password_(std::move(password)) {
}

EncryptionResult CryptoEngine::encrypt(const std::vector<uint8_t>& plaintext) {
    EncryptionResult result;

    result.params.salt = generate_salt();
    auto iv_bytes = random_bytes(IV_SIZE);
    std::copy(iv_bytes.begin(), iv_bytes.end(), result.params.iv.begin());

    auto derived_key = derive_key(password_, result.params.salt);

    result.ciphertext.resize(plaintext.size());

    mbedtls_gcm_context ctx;
    mbedtls_gcm_init(&ctx);

    int ret = mbedtls_gcm_setkey(
        &ctx,
        MBEDTLS_CIPHER_ID_AES,
        derived_key.data(),
        KEY_SIZE * 8
    );

    if (ret != 0) {
        std::memset(derived_key.data(), 0, derived_key.size());
        mbedtls_gcm_free(&ctx);
        throw std::runtime_error("Failed to set GCM key");
    }

    size_t out_len = 0;
    ret = mbedtls_gcm_crypt_and_tag(
        &ctx,
        MBEDTLS_GCM_ENCRYPT,
        plaintext.size(),
        result.params.iv.data(),
        IV_SIZE,
        nullptr, 0,
        plaintext.data(),
        result.ciphertext.data(),
        TAG_SIZE,
        result.params.tag.data()
    );

    mbedtls_gcm_free(&ctx);

    if (ret != 0) {
        std::memset(derived_key.data(), 0, derived_key.size());
        throw std::runtime_error("Encryption failed");
    }

    result.ciphertext.resize(plaintext.size());
    std::memset(derived_key.data(), 0, derived_key.size());

    return result;
}

std::vector<uint8_t> CryptoEngine::decrypt(
    const std::vector<uint8_t>& ciphertext,
    const CryptoParams& params
) {
    auto derived_key = derive_key(password_, params.salt);

    std::vector<uint8_t> plaintext(ciphertext.size());

    mbedtls_gcm_context ctx;
    mbedtls_gcm_init(&ctx);

    int ret = mbedtls_gcm_setkey(
        &ctx,
        MBEDTLS_CIPHER_ID_AES,
        derived_key.data(),
        KEY_SIZE * 8
    );

    if (ret != 0) {
        std::memset(derived_key.data(), 0, derived_key.size());
        mbedtls_gcm_free(&ctx);
        throw std::runtime_error("Failed to set GCM key");
    }

    size_t out_len = 0;
    ret = mbedtls_gcm_auth_decrypt(
        &ctx,
        ciphertext.size(),
        params.iv.data(),
        IV_SIZE,
        nullptr, 0,
        params.tag.data(),
        TAG_SIZE,
        ciphertext.data(),
        plaintext.data()
    );

    mbedtls_gcm_free(&ctx);

    if (ret != 0) {
        std::memset(derived_key.data(), 0, derived_key.size());
        throw std::runtime_error("Decryption failed: authentication error");
    }

    plaintext.resize(ciphertext.size());
    std::memset(derived_key.data(), 0, derived_key.size());

    return plaintext;
}

std::vector<uint8_t> CryptoEngine::encrypt_with_params(
    const std::vector<uint8_t>& plaintext,
    const CryptoParams& params
) {
    EncryptionResult result;
    result.params = params;

    auto derived_key = derive_key(password_, params.salt);

    result.ciphertext.resize(plaintext.size());

    mbedtls_gcm_context ctx;
    mbedtls_gcm_init(&ctx);

    int ret = mbedtls_gcm_setkey(
        &ctx,
        MBEDTLS_CIPHER_ID_AES,
        derived_key.data(),
        KEY_SIZE * 8
    );

    if (ret != 0) {
        std::memset(derived_key.data(), 0, derived_key.size());
        mbedtls_gcm_free(&ctx);
        throw std::runtime_error("Failed to set GCM key");
    }

    size_t out_len = 0;
    ret = mbedtls_gcm_crypt_and_tag(
        &ctx,
        MBEDTLS_GCM_ENCRYPT,
        plaintext.size(),
        params.iv.data(),
        IV_SIZE,
        nullptr, 0,
        plaintext.data(),
        result.ciphertext.data(),
        TAG_SIZE,
        result.params.tag.data()
    );

    mbedtls_gcm_free(&ctx);

    if (ret != 0) {
        std::memset(derived_key.data(), 0, derived_key.size());
        throw std::runtime_error("Encryption failed");
    }

    result.ciphertext.resize(plaintext.size());
    std::memset(derived_key.data(), 0, derived_key.size());

    return result.ciphertext;
}

std::vector<uint8_t> CryptoEngine::decrypt_with_key(
    const std::vector<uint8_t>& ciphertext,
    const std::array<uint8_t, KEY_SIZE>& key,
    const std::array<uint8_t, IV_SIZE>& iv,
    const std::array<uint8_t, TAG_SIZE>& tag
) {
    std::vector<uint8_t> plaintext(ciphertext.size());

    mbedtls_gcm_context ctx;
    mbedtls_gcm_init(&ctx);

    int ret = mbedtls_gcm_setkey(
        &ctx,
        MBEDTLS_CIPHER_ID_AES,
        key.data(),
        KEY_SIZE * 8
    );

    if (ret != 0) {
        mbedtls_gcm_free(&ctx);
        throw std::runtime_error("Failed to set GCM key");
    }

    size_t out_len = 0;
    ret = mbedtls_gcm_auth_decrypt(
        &ctx,
        ciphertext.size(),
        iv.data(),
        IV_SIZE,
        nullptr, 0,
        tag.data(),
        TAG_SIZE,
        ciphertext.data(),
        plaintext.data()
    );

    mbedtls_gcm_free(&ctx);

    if (ret != 0) {
        throw std::runtime_error("Decryption failed: authentication error");
    }

    plaintext.resize(ciphertext.size());
    return plaintext;
}

} // namespace txtcrypt
