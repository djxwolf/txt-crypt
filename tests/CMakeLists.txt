find_package(Threads REQUIRED)

add_executable(txt_crypt_tests
    unit/test_base64.cpp
    unit/test_kdf.cpp
    unit/test_crypto_engine.cpp
    unit/test_file_format.cpp
    unit/test_temp_file.cpp
    unit/test_integration.cpp
)

target_include_directories(txt_crypt_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/tests
    ${MBEDTLS_INCLUDE_DIRS}
)

target_link_libraries(txt_crypt_tests PRIVATE
    ${MBEDTLS_LIBRARIES}
    Threads::Threads
)

add_test(NAME txt_crypt_unit_tests COMMAND txt_crypt_tests)
