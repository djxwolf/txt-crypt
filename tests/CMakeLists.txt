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
