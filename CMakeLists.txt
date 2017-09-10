cmake_minimum_required(VERSION 3.6)
set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
set(CMAKE_CXX_FLAGS "-g")
set(CMAKE_CXX_STANDARD 11)

project(ssl)

set(SOURCE_FILES
    src/Main.cc
    src/socket.h
    src/plain_socket.cc
    src/plain_socket.h
    src/tls_socket.cc
    src/tls_socket.h
    src/ssl_manager.cc
    src/ssl_manager.h
    )

add_executable(main ${SOURCE_FILES})

target_link_libraries(main ssl crypto)
