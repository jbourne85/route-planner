find_package(gtest REQUIRED)

file(GLOB MESSAGE_TESTS "messages/test_*.cpp")
add_executable(runTests ${MESSAGE_TESTS})
target_include_directories(runTests PUBLIC ${PROJECT_SOURCE_DIR}/include)
include_directories(${GTest_INCLUDE_DIR})

target_link_libraries(runTests GTest::gtest GTest::gtest_main pthread)

add_test(NAME runTests COMMAND runTests)
