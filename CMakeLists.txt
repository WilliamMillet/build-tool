cmake_minimum_required(VERSION 3.20)

project(MyMake LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(
    -g
    -O0
    -fsanitize=address,undefined
    -Werror
    -Wextra
    -Wall
)

add_link_options(-fsanitize=address,undefined)

# Application

file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS "src/*.cpp")

add_executable(my_make ${APP_SOURCES})

target_include_directories(my_make PRIVATE include)


# Tests

file(GLOB_RECURSE  TEST_SOURCES CONFIGURE_DEPENDS "tests/*.cpp")

# Remove main.cpp from app sources for tests
list(REMOVE_ITEM APP_SOURCES "${CMAKE_SOURCE_DIR}/src/main.cpp")

add_executable(my_make_tests
    ${TEST_SOURCES}
    ${APP_SOURCES}
)

target_include_directories(my_make_tests PRIVATE
    include
    ${CMAKE_SOURCE_DIR}
)
