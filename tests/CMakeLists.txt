find_package(Threads REQUIRED)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Secure String Tests
add_executable(test_secure_memory
    unit/test_secure_memory_minimal.cpp
)

target_include_directories(test_secure_memory PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests
    ${CMAKE_SOURCE_DIR}/tests/catch2
    ${MBEDTLS_INCLUDE_DIRS}
    /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include/c++/v1
)

target_link_libraries(test_secure_memory PRIVATE
    ${MBEDTLS_LIBRARIES}
    Threads::Threads
    stdc++
)

add_test(NAME test_secure_memory COMMAND test_secure_memory)

# File Format Tests
add_executable(test_file_format
    unit/test_file_format_minimal.cpp
)

target_include_directories(test_file_format PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests
    ${CMAKE_SOURCE_DIR}/tests/catch2
    ${MBEDTLS_INCLUDE_DIRS}
    /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include/c++/v1
)

target_link_libraries(test_file_format PRIVATE
    ${MBEDTLS_LIBRARIES}
    Threads::Threads
    stdc++
)

add_test(NAME test_file_format COMMAND test_file_format)

# Base64 Tests
add_executable(test_base64
    unit/test_base64.cpp
    ../src/io/base64.cpp
)

target_include_directories(test_base64 PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests
    ${CMAKE_SOURCE_DIR}/tests/catch2
    ${MBEDTLS_INCLUDE_DIRS}
    /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include/c++/v1
)

target_link_libraries(test_base64 PRIVATE
    ${MBEDTLS_LIBRARIES}
    Threads::Threads
    stdc++
)

add_test(NAME test_base64 COMMAND test_base64)

# KDF Tests
add_executable(test_kdf
    unit/test_kdf.cpp
    ../src/core/kdf.cpp
)

target_include_directories(test_kdf PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests
    ${CMAKE_SOURCE_DIR}/tests/catch2
    ${MBEDTLS_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/third_party/mbedtls/tf-psa-crypto/drivers/builtin/include
    /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include/c++/v1
)

target_link_libraries(test_kdf PRIVATE
    ${MBEDTLS_LIBRARIES}
    Threads::Threads
    stdc++
)

add_test(NAME test_kdf COMMAND test_kdf)
