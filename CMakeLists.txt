cmake_minimum_required(VERSION 2.8)

project(zCUT)

set(CMAKE_C_FLAGS "-Wall -Wextra -Werror")

add_subdirectory(lib)
add_subdirectory(tests)
