cmake_minimum_required(VERSION 3.13)
project(metro C)

set(CMAKE_C_STANDARD 99)
SET(MAJOR_VERSION "0")
SET(MINOR_VERSION "1")
SET(PATCH_VERSION "0")



add_executable(metro metro.c)

target_link_libraries(metro m)
target_link_libraries(metro openal)
target_link_libraries(metro audio)
