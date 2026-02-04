cmake_minimum_required(VERSION 3.12)
project(eval CXX)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenSSL REQUIRED)

add_executable(eval src/main.cpp)

target_include_directories(eval PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/protocol
    /usr/local/include/
    /usr/local/include/cryptoTools/
)

target_link_libraries(eval 
    OpenSSL::SSL 
    OpenSSL::Crypto
    /usr/local/lib/libcryptoTools.a
    pthread
)

target_compile_options(eval PRIVATE -msse2 -mpclmul -maes)

add_executable(test_cmp src/test_cmp.cpp)

target_include_directories(test_cmp PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/protocol
    /usr/local/include/
    /usr/local/include/cryptoTools/
)

target_link_libraries(test_cmp 
    OpenSSL::SSL 
    OpenSSL::Crypto
    /usr/local/lib/libcryptoTools.a
    pthread
)

target_compile_options(test_cmp PRIVATE -msse2 -mpclmul -maes)

add_executable(test_cot src/test_cot.cpp)

target_include_directories(test_cot PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/protocol
    /usr/local/include/
    /usr/local/include/cryptoTools/
)

target_link_libraries(test_cot 
    OpenSSL::SSL 
    OpenSSL::Crypto
    /usr/local/lib/libcryptoTools.a
    pthread
)

target_compile_options(test_cot PRIVATE -msse2 -mpclmul -maes)