cmake_minimum_required(VERSION 3.20.5)

project(emu)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY build)

include_directories(include)

add_executable(${PROJECT_NAME} src/main.cpp src/cpu.cpp src/memory.cpp src/system.cpp)
target_link_libraries(${PROJECT_NAME} -pthread -O3)

add_executable(${PROJECT_NAME}_tests src/tests.cpp src/cpu.cpp src/memory.cpp src/system.cpp)
target_link_libraries(${PROJECT_NAME}_tests -pthread -g)
