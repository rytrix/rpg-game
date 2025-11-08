cmake_minimum_required(VERSION 3.22.0)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(_DEBUG)
endif()

project(rpg)
add_executable(${PROJECT_NAME} 
    src/main.cpp 
    src/gear.cpp 
    src/character.cpp
)

target_precompile_headers(${PROJECT_NAME} PRIVATE src/pch.hpp)

add_subdirectory(src/sqlite)
# add_subdirectory(src/raylib)
add_subdirectory(src/sdl3)
add_subdirectory(src/glad)

target_link_libraries(${PROJECT_NAME} 
    sqlite 
    # raylib 
    SDL3-static
    glad
)

target_include_directories(${PROJECT_NAME} 
    PUBLIC src/SDL3/include
    PUBLIC src/glad/include
)
