add_executable(client client.cpp ${MESSAGE_SOURCES} ${TCP_SOURCES} ${ROUTE_SOURCES})
target_include_directories(client PUBLIC ${PROJECT_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
target_link_libraries(client Threads::Threads log4cxx)
set_target_properties(client PROPERTIES CXX_STANDARD 17)

add_executable(server server.cpp ${MESSAGE_SOURCES} ${TCP_SOURCES} ${ROUTE_SOURCES})
target_include_directories(server PUBLIC ${PROJECT_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
target_link_libraries(server Threads::Threads log4cxx)
set_target_properties(server PROPERTIES CXX_STANDARD 17)