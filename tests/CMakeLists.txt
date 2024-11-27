
find_package(GTest REQUIRED)

file(GLOB MESSAGE_TESTS "messages/*.cpp")
add_executable(runTests ${MESSAGE_TESTS})
target_include_directories(runTests PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(runTests GTest::GTest GTest::Main pthread)

add_test(NAME runTests COMMAND runTests)