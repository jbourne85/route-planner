cmake_minimum_required(VERSION 3.13)
project(route-planner VERSION 0.1.0 LANGUAGES C CXX)
set (CMAKE_CXX_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(ROUTE_PLANNER_SRC_ROOT ${CMAKE_SOURCE_DIR}/src)

find_package(Boost REQUIRED)
find_package(Threads REQUIRED)
find_package(log4cxx REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

set(MESSAGE_SOURCES 
    "${ROUTE_PLANNER_SRC_ROOT}/messages/MsgFactory.cpp"
)
set(TCP_SOURCES 
    "${ROUTE_PLANNER_SRC_ROOT}/comms/TcpClient.cpp"
    "${ROUTE_PLANNER_SRC_ROOT}/comms/TcpServer.cpp"
    "${ROUTE_PLANNER_SRC_ROOT}/comms/TcpMsgMatch.cpp"
)
set(ROUTE_SOURCES
    "${ROUTE_PLANNER_SRC_ROOT}/route/Location.cpp"
    "${ROUTE_PLANNER_SRC_ROOT}/route/FileLocationDatabase.cpp"
    "${ROUTE_PLANNER_SRC_ROOT}/route/FileRouteDatabase.cpp"
    "${ROUTE_PLANNER_SRC_ROOT}/route/RoutePlanner.cpp"
)

enable_testing()
add_subdirectory(src)
add_subdirectory(tests)
