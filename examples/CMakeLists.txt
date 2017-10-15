include_directories(src)

add_executable(http_request request.cc)
add_executable(echo_server echo_server.cc)
add_executable(base64 base64.cc)
add_executable(websocket websocket.cc)

target_link_libraries(http_request cmdsock)
target_link_libraries(echo_server cmdsock pthread)
target_link_libraries(base64 cmdsock)
target_link_libraries(websocket cmdsock)

