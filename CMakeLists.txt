cmake_minimum_required(VERSION 3.17)

project(CHIP-8)

set(CMAKE_CXX_STANDARD 17)
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

if (MSVC)
    target_compile_options(CHIP-8 PUBLIC /Wall)
else ()
    target_compile_options(CHIP-8 PUBLIC ${WARNINGS})
endif ()

add_executable(tests tests/main.cpp)
if (MSVC)
    target_compile_options(CHIP-8 PUBLIC /W3 /wd5045)
else ()
    target_compile_options(CHIP-8 PUBLIC ${WARNINGS})
endif ()
target_include_directories(tests PUBLIC src)