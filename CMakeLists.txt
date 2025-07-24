cmake_minimum_required(VERSION 3.12)
project(eval CXX)

set(CMAKE_CXX_STANDARD 11)


list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew")
find_package(OpenSSL REQUIRED)

add_executable(eval src/main.cpp)

target_include_directories(eval PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/protocol
)

target_link_libraries(eval OpenSSL::SSL OpenSSL::Crypto)