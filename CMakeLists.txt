cmake_minimum_required(VERSION 3.6)
set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
set(CMAKE_CXX_FLAGS "-g")
set(CMAKE_CXX_STANDARD 11)

project(Botan)

include_directories(/usr/include/botan-2)
include_directories(/usr/local/include/botan-2)

set(SOURCE_FILES
    src/Main.cc
    src/socket.cc
    src/socket.h)

add_executable(main ${SOURCE_FILES})

target_link_libraries(main botan-2 m)
