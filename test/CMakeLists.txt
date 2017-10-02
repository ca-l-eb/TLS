set(CMAKE_CXX_STANDARD 11)

find_package(GTest REQUIRED)

add_executable(test_base64 base64.cc)
target_include_directories(test_base64 PUBLIC src ${GTEST_INCLUDE_DIRS})

target_link_libraries(test_base64 cmdsock ${GTEST_LIBRARIES})
