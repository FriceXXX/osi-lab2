cmake_minimum_required(VERSION 3.10)
project(ProcessCommunication VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(WIN32)
    add_compile_definitions(_WIN32_WINNT=0x0601)
else()
    add_compile_options(-Wall -Wextra -D_POSIX_C_SOURCE=200809L)
endif()

add_executable(parent parent.cpp common.hpp)

add_executable(child child.cpp common.hpp)

target_include_directories(parent PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(child PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(parent PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set_target_properties(child PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})