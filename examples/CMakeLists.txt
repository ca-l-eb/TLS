set(CMAKE_CXX_STANDARD 11)

add_executable(http_request request.cc)
target_include_directories(http_request PUBLIC src)

add_executable(echo_server echo_server.cc)
target_include_directories(echo_server PUBLIC src)

add_executable(base64 base64.cc)
target_include_directories(base64 PUBLIC src)

add_executable(websocket websocket.cc)
target_include_directories(websocket PUBLIC src)

target_link_libraries(http_request cmdsock)
target_link_libraries(echo_server cmdsock pthread)
target_link_libraries(base64 cmdsock)
target_link_libraries(websocket cmdsock)

