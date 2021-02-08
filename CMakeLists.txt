cmake_minimum_required(VERSION 3.5)

project(ports_checker)

add_executable(${PROJECT_NAME} src/swarm.cpp src/main.cpp)

target_link_libraries(${PROJECT_NAME} pthread)

