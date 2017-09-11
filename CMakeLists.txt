cmake_minimum_required(VERSION 3.6)
set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
set(CMAKE_CXX_FLAGS "-g")
set(CMAKE_CXX_STANDARD 11)

project(ssl)

set(SOURCE_FILES
    src/main.cc
    src/socket.h
    src/plain_socket.cc
    src/plain_socket.h
    src/tls_socket.cc
    src/tls_socket.h
    src/ssl_manager.cc
    src/ssl_manager.h
    src/tokenizer.cc
    src/tokenizer.h
    src/http_request.cc
    src/http_request.h
    src/http_response.cc
    src/http_response.h
    src/stream.cc
    src/stream.h
    )

add_executable(main ${SOURCE_FILES})

target_link_libraries(main ssl crypto)
