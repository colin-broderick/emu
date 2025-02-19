cmake_minimum_required(VERSION 3.22.1)

project(emu)

set(CMAKE_CXX_STANDARD 23)

include_directories(include)

add_executable(${PROJECT_NAME} src/main.cpp src/cpu.cpp src/memory.cpp src/system.cpp src/cpu_flag_setters.cpp)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -O3 -Wextra -Werror -Wshadow -Wpedantic -Wconversion)
