set(CMAKE_CXX_STANDARD 11)

find_package(GTest REQUIRED)

add_executable(test_base64 base64.cc)
target_include_directories(test_base64 PUBLIC src ${GTEST_INCLUDE_DIRS})

add_executable(exception_test exception_test.cc)
target_include_directories(exception_test PUBLIC src ${GTEST_INCLUDE_DIRS})

add_executable(resource_parse_test resource_parse_test.cc)
target_include_directories(resource_parse_test PUBLIC src ${GTEST_INCLUDE_DIRS})

target_link_libraries(test_base64 cmdsock ${GTEST_LIBRARIES})
target_link_libraries(exception_test cmdsock ${GTEST_LIBRARIES})
target_link_libraries(resource_parse_test cmdsock ${GTEST_LIBRARIES})
