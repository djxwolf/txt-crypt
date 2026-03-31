#include "core/kdf.h"
#include <stdexcept>
#include <cstring>

#include "mbedtls/private/entropy.h"
#include "mbedtls/private/ctr_drbg.h"
#include "mbedtls/private/pkcs5.h"
#include "mbedtls/md.h"

namespace txtcrypt {

namespace {
thread_local mbedtls_entropy_context entropy;
thread_local mbedtls_ctr_drbg_context drbg;
thread_local bool rng_initialized = false;

void init_rng() {
    if (!rng_initialized) {
        mbedtls_entropy_init(&entropy);
        mbedtls_ctr_drbg_init(&drbg);

        const char* pers = "txt_crypt_rng";
        if (mbedtls_ctr_drbg_seed(&drbg, mbedtls_entropy_func, &entropy,
                                  reinterpret_cast<const unsigned char*>(pers),
                                  std::strlen(pers)) != 0) {
            throw std::runtime_error("Failed to initialize RNG");
        }
        rng_initialized = true;
    }
}
}

std::array<uint8_t, KEY_SIZE> derive_key(
    const SecureString& password,
    const std::array<uint8_t, SALT_SIZE>& salt,
    const KdfParams& params
) {
    std::array<uint8_t, KEY_SIZE> key;

    auto pwd_view = password.view();

    int ret = mbedtls_pkcs5_pbkdf2_hmac_ext(
        MBEDTLS_MD_SHA256,
        reinterpret_cast<const unsigned char*>(pwd_view.data()),
        pwd_view.size(),
        salt.data(),
        salt.size(),
        params.t_cost * 10000,
        KEY_SIZE,
        key.data()
    );

    if (ret != 0) {
        throw std::runtime_error("Key derivation failed");
    }

    return key;
}

std::array<uint8_t, SALT_SIZE> generate_salt() {
    std::array<uint8_t, SALT_SIZE> salt;
    auto bytes = random_bytes(SALT_SIZE);
    std::copy(bytes.begin(), bytes.end(), salt.begin());
    return salt;
}

std::vector<uint8_t> random_bytes(size_t size) {
    init_rng();

    std::vector<uint8_t> bytes(size);
    int ret = mbedtls_ctr_drbg_random(&drbg, bytes.data(), size);
    if (ret != 0) {
        throw std::runtime_error("Random number generation failed");
    }
    return bytes;
}

} // namespace txtcrypt
