find_package(gtest REQUIRED)
set(TEST_DATA_DIR ${CMAKE_SOURCE_DIR}/test_data)

add_custom_target(copy_test_data ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${TEST_DATA_DIR} ${CMAKE_BINARY_DIR}/tests/test_data
)

add_custom_target(copy_test_log4cxx_config ALL
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/log4cxx.properties ${CMAKE_BINARY_DIR}/tests/log4cxx.properties
)

file(GLOB MESSAGE_TESTS "**/test_*.cpp")

add_executable(runTests ${MESSAGE_TESTS} ${MESSAGE_SOURCES} ${TCP_SOURCES} ${ROUTE_SOURCES})
target_include_directories(runTests PUBLIC ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/tests)
include_directories(${GTest_INCLUDE_DIR})
add_dependencies(runTests copy_test_data)

target_link_libraries(runTests GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main pthread log4cxx)

add_test(NAME runTests COMMAND runTests)