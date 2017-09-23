cmake_minimum_required(VERSION 3.6)
set(CMAKE_CXX_FLAGS "-g")
set(CMAKE_CXX_STANDARD 11)

project(mysocket)

link_directories(/usr/local/lib)
find_package(OpenSSL REQUIRED)

set(SOURCE_FILES
    src/http_request.cc
    src/http_response.cc
    src/http_pool.cc
    src/tcp_socket.cc
    src/ssl_manager.cc
    src/stream.cc
    src/tcp_server.cc
    src/tls_server.cc
    src/tls_socket.cc
    src/tokenizer.cc
    )

set(HEADER_FILES
    src/http_request.h
    src/http_response.h
    src/http_pool.h
    src/tcp_socket.h
    src/server_socket.h
    src/socket.h
    src/ssl_manager.h
    src/stream.h
    src/tls_server.h
    src/tls_socket.h
    src/tcp_server.h
    src/tokenizer.h
    )

set(MAIN src/main.cc)

include_directories(${OPENSSL_INCLUDE_DIR})

add_library(cmdsock SHARED ${SOURCE_FILES})
add_executable(main ${MAIN})

target_link_libraries(main ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY} cmdsock)
target_link_libraries(cmdsock ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY})
