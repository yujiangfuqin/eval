cmake_minimum_required(VERSION 3.12)
project(eval CXX)

set(CMAKE_CXX_STANDARD 20)

list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew")
find_package(OpenSSL REQUIRED)

# 设置libOTe相关路径
set(LIBOTE_ROOT "/Users/ling/Documents/code/libOTe")
set(LIBOTE_BUILD_DIR "${LIBOTE_ROOT}/out/build/osx")

add_executable(eval src/main.cpp)

target_include_directories(eval PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/protocol
    ${LIBOTE_ROOT}/cryptoTools
    ${LIBOTE_BUILD_DIR}/cryptoTools
)

target_link_libraries(eval 
    OpenSSL::SSL 
    OpenSSL::Crypto
    ${LIBOTE_BUILD_DIR}/cryptoTools/cryptoTools/libcryptoTools.a
    pthread
)