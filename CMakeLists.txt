cmake_minimum_required(VERSION 2.8)

project(zCUT)

find_program(CLANG "clang")
if (EXISTS ${CLANG})
    set(CMAKE_C_COMPILER "clang")
endif()

set(CMAKE_C_FLAGS "-std=gnu89 -Wall -Wextra -Werror -Wno-format-security")

add_subdirectory(lib)
add_subdirectory(tests)
