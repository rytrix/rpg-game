cmake_minimum_required(VERSION 3.22.0)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(_DEBUG)
endif()

project(glad)
add_library(${PROJECT_NAME} 
    src/glad.c
)

target_include_directories(${PROJECT_NAME} 
    PRIVATE include
)
