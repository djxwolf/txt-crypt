cmake_minimum_required(VERSION 3.15)
project(txt-crypt VERSION 0.2.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTS "Build tests" OFF)
option(ENABLE_THREADS "Enable threading" ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Find mbedtls
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(MBEDTLS REQUIRED mbedtls)
endif()

# If not found, use bundled
if(NOT MBEDTLS_FOUND)
    set(MBEDTLS_DIR ${CMAKE_SOURCE_DIR}/third_party/mbedtls)
    if(NOT EXISTS ${MBEDTLS_DIR})
        message(FATAL_ERROR "mbedtls not found. Run: git submodule update --init --recursive")
    endif()
    set(MBEDTLS_INCLUDE_DIRS ${MBEDTLS_DIR}/include)
    set(MBEDTLS_LIBRARIES mbedtls mbedx509 mbedcrypto)
    add_subdirectory(${MBEDTLS_DIR} ${CMAKE_BINARY_DIR}/mbedtls)

    # Create alias for backward compatibility
    if(TARGET tfpsacrypto AND NOT TARGET mbedcrypto)
        add_library(mbedcrypto ALIAS tfpsacrypto)
    endif()
endif()

# Main executable
add_executable(txt-crypt
    src/main.cpp
    src/cli/cli_parser.cpp
    src/cli/progress.cpp
    src/core/stream_processor.cpp
    src/core/crypto_engine.cpp
    src/core/kdf.cpp
    src/io/file_reader.cpp
    src/io/file_writer.cpp
    src/io/base64.cpp
    src/utils/error_codes.cpp
    src/utils/temp_file.cpp
)

target_include_directories(txt-crypt PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${MBEDTLS_INCLUDE_DIRS}
    /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include/c++/v1
)

find_package(Threads REQUIRED)
target_link_libraries(txt-crypt PRIVATE
    ${MBEDTLS_LIBRARIES}
    Threads::Threads
)

if(ENABLE_THREADS)
    target_compile_definitions(txt-crypt PRIVATE TXTCRYPT_ENABLE_THREADS)
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

install(TARGETS txt-crypt DESTINATION bin)
