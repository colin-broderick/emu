cmake_minimum_required(VERSION 3.22.1)

project(emu)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(GTest REQUIRED)

include_directories(include)

add_executable(${PROJECT_NAME}_test src/test_main.cpp src/rewrite.cpp)
target_compile_options(${PROJECT_NAME}_test PRIVATE -Wall -g -Wextra -Werror -Wshadow -Wpedantic -Wconversion)
target_link_libraries(${PROJECT_NAME}_test GTest::gtest_main)

add_executable(${PROJECT_NAME} src/main.cpp src/rewrite.cpp)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -g -Wextra -Werror -Wshadow -Wpedantic -Wconversion)
