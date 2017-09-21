cmake_minimum_required(VERSION 3.6)
set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
set(CMAKE_CXX_FLAGS "-g")
set(CMAKE_CXX_STANDARD 11)

project(mysocket)

set(SOURCE_FILES
    src/http_request.cc
    src/http_response.cc
    src/http_pool.cc
    src/plain_socket.cc
    src/ssl_manager.cc
    src/stream.cc
    src/tls_socket.cc
    src/tokenizer.cc
    )

set(HEADER_FILES
    src/http_request.h
    src/http_response.h
    src/http_pool.h
    src/plain_socket.h
    src/socket.h
    src/ssl_manager.h
    src/stream.h
    src/tls_socket.h
    src/tokenizer.h
    )

set(MAIN src/main.cc)

add_library(cmdsock SHARED ${SOURCE_FILES})
add_executable(main ${MAIN})

target_link_libraries(main ssl crypto cmdsock)
target_link_libraries(cmdsock ssl crypto)
