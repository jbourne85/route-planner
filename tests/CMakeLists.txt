find_package(gtest REQUIRED)
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

file(GLOB MESSAGE_TESTS "**/test_*.cpp")

add_executable(runTests ${MESSAGE_TESTS} ${MESSAGE_SOURCES} ${TCP_SOURCES})
target_include_directories(runTests PUBLIC ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/tests)
include_directories(${GTest_INCLUDE_DIR})

target_link_libraries(runTests GTest::gtest GTest::gtest_main pthread)

add_test(NAME runTests COMMAND runTests)
