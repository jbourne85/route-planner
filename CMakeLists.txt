cmake_minimum_required(VERSION 3.5.0)
project(route-planner VERSION 0.1.0 LANGUAGES C CXX)

add_executable(route-planner main.cpp)

include(CTest)
enable_testing()

