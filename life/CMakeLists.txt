cmake_minimum_required(VERSION 3.10)
project(life CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(USE_SDL "Enable SDL2 visualization" ON)

set(SOURCES
    src/main.cpp
    src/Field.cpp
    src/Generator.cpp
    src/Stepper.cpp
)

if(USE_SDL)
    find_package(SDL2 QUIET)
    if(SDL2_FOUND)
        message(STATUS "SDL2 found — visualization enabled")
        list(APPEND SOURCES src/Visualizer.cpp)
        add_compile_definitions(USE_SDL)
    else()
        message(STATUS "SDL2 not found — visualization disabled")
    endif()
endif()

add_executable(life ${SOURCES})
target_include_directories(life PRIVATE src)

find_package(Threads REQUIRED)
target_link_libraries(life PRIVATE Threads::Threads)

if(USE_SDL AND SDL2_FOUND)
    target_link_libraries(life PRIVATE SDL2::SDL2)
endif()

if(MSVC)
    target_compile_options(life PRIVATE /W4 /O2)
else()
    target_compile_options(life PRIVATE -Wall -Wextra -O2)
endif()
