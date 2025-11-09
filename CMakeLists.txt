cmake_minimum_required(VERSION 3.22.0)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(_DEBUG)
endif()

project(rpg)
add_executable(${PROJECT_NAME} 
    src/main.cpp 
    src/game_logic/gear.cpp 
    src/game_logic/character.cpp
)

target_precompile_headers(${PROJECT_NAME} PRIVATE src/pch.hpp)

add_subdirectory(dep/sqlite)
add_subdirectory(src/renderer)
# add_subdirectory(dep/raylib)
# add_subdirectory(dep/sdl3)
# add_subdirectory(dep/glad)

target_link_libraries(${PROJECT_NAME} 
    sqlite 
    renderer
    # raylib 
    # SDL3-static
    # glad
)

target_include_directories(${PROJECT_NAME} 
    PUBLIC src/renderer/include
    PUBLIC dep
    # PUBLIC src/SDL3/include
    # PUBLIC src/glad/include
)
