cmake_minimum_required(VERSION 3.0)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra")

project(cmdsock)

add_subdirectory(test)
add_subdirectory(examples)

link_directories(/usr/local/lib)
find_package(OpenSSL REQUIRED)

set(SOURCE_FILES
    src/base64.cc
    src/http_pool.cc
    src/http_request.cc
    src/http_response.cc
    src/resource_parser.cc
    src/resource_parser.h
    src/server_socket.cc
    src/ssl_manager.cc
    src/stream.cc
    src/tcp_server.cc
    src/tcp_socket.cc
    src/tls_server.cc
    src/tls_socket.cc
    src/tokenizer.cc
    src/udp_socket.cc
    src/websocket.cc
)

set(HEADER_FILES
    src/base64.h
    src/http_pool.h
    src/http_request.h
    src/http_response.h
    src/resource_parser.cc
    src/resource_parser.h
    src/server_socket.h
    src/socket.h
    src/ssl_manager.h
    src/stream.h
    src/string_utils.h
    src/tcp_server.h
    src/tcp_socket.h
    src/tls_server.h
    src/tls_socket.h
    src/tokenizer.h
    src/udp_socket.h
    src/websocket.h
    src/exceptions.h)

add_library(cmdsock SHARED ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories(cmdsock PUBLIC ${OPENSSL_INCLUDE_DIR} src)
target_link_libraries(cmdsock ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY})
