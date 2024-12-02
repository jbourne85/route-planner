cmake_minimum_required(VERSION 3.13)
project(route-planner VERSION 0.1.0 LANGUAGES C CXX)

set(ROUTE_PLANNER_SRC_ROOT ${CMAKE_SOURCE_DIR}/src)

enable_testing()
add_subdirectory(src)
add_subdirectory(tests)
