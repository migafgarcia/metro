cmake_minimum_required(VERSION 3.13)
project(metro C)

set(CMAKE_C_STANDARD 99)

add_executable(metro metro.c)

target_link_libraries(metro m)
target_link_libraries(metro openal)