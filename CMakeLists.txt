cmake_minimum_required(VERSION  3.20)

project(builder LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_MODULE_STD ON)

add_compile_options(
    -g
    -O0
    -fsanitize=address,undefined
    -Werror
    -Wextra
    -Wall
)

add_link_options(-fsanitize=address,undefined)

file(GLOB ALL_SOURCES "src/*.cpp")

add_executable(my_make ${ALL_SOURCES})