cmake_minimum_required(VERSION 3.22.0)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(_DEBUG)
endif()

project(stb_image)

add_library(${PROJECT_NAME} 
    src/stb_image_implementation.cpp
)

target_include_directories(${PROJECT_NAME} 
    PRIVATE include
)
