cmake_minimum_required(VERSION 3.6)
set(CMAKE_CXX_FLAGS "-g -Wall")
set(CMAKE_CXX_STANDARD 11)

project(cmdsock)

link_directories(/usr/local/lib)
find_package(OpenSSL REQUIRED)

set(SOURCE_FILES
    src/base64.cc
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
    src/base64.h
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


add_library(cmdsock SHARED ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories(cmdsock PUBLIC ${OPENSSL_INCLUDE_DIR} src)
target_link_libraries(cmdsock ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY})

# -------------EXAMPLES---------------
add_executable(http_request examples/request.cc)
target_include_directories(http_request PUBLIC src)

add_executable(echo_server examples/echo_server.cc)
target_include_directories(echo_server PUBLIC src)

add_executable(base64 examples/base64.cc)
target_include_directories(base64 PUBLIC src)

target_link_libraries(http_request cmdsock)
target_link_libraries(echo_server cmdsock pthread)
target_link_libraries(base64 cmdsock)

# -------------EXAMPLES---------------
