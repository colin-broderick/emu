cmake_minimum_required(VERSION 3.16)

project(emu)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY build)

include_directories(include)

add_executable(${PROJECT_NAME} src/main.cpp src/cpu.cpp src/memory.cpp src/system.cpp)
target_link_libraries(${PROJECT_NAME} -pthread)
