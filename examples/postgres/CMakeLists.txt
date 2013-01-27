cmake_minimum_required(VERSION 2.8)


add_definitions(-std=c++11)

add_executable(blob select.cpp insert.cpp main.cpp)

target_link_libraries(blob mysqlclient ssl crypto)