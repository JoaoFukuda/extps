cmake_minimum_required(VERSION 3.5)

project(extps)

add_executable(${PROJECT_NAME} src/net.cpp src/swarm.cpp src/main.cpp)

target_link_libraries(${PROJECT_NAME} pthread)

