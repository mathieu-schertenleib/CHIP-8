cmake_minimum_required(VERSION 3.17)

project(CHIP-8)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED On)

set(WARNINGS
        -Wall
        -Wextra
        -Wpedantic
        -Wnarrowing
        -Wconversion
        -Wdouble-promotion
        -Wduplicated-cond
        -Woverloaded-virtual
        -Wnon-virtual-dtor)


add_executable(CHIP-8 src/main.cpp src/interpreter.cpp src/interpreter.hpp)
target_compile_options(CHIP-8 PUBLIC ${WARNINGS})


add_executable(tests tests/main.cpp)
target_compile_options(tests PUBLIC ${WARNINGS})
target_include_directories(tests PUBLIC src)