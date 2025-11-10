cmake_minimum_required(VERSION 3.22.0)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(_DEBUG)
endif()

project(rpg)
add_executable(${PROJECT_NAME} 
    src/main.cpp 

    src/game_logic/gear.cpp 
    src/game_logic/character.cpp

	src/renderer/window.cpp
	src/renderer/buffer.cpp
	src/renderer/vertex.cpp
	src/renderer/shader.cpp
	src/renderer/texture.cpp
	src/renderer/camera.cpp
    src/renderer/framebuffer.cpp
)

target_precompile_headers(${PROJECT_NAME} PUBLIC src/pch.hpp)

add_subdirectory(dep/sqlite)
add_subdirectory(dep/glad)
add_subdirectory(dep/stb_image)
add_subdirectory(dep/sdl3)
add_subdirectory(dep/assimp)

target_link_libraries(${PROJECT_NAME} 
    sqlite 
	glad
	stb_image
	SDL3::SDL3
)

target_include_directories(${PROJECT_NAME} 
    PUBLIC src/renderer/include
    PUBLIC dep
	PUBLIC dep/glad/include/
	PUBLIC dep/stb/include
	PUBLIC dep/sdl3/include
	PUBLIC dep/glm/
)
