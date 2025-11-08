cmake_minimum_required(VERSION 3.22.0)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(_DEBUG)
endif()

project(renderer)

add_library(${PROJECT_NAME}
	src/window.cpp
	src/buffer.cpp
	src/vertex.cpp
	src/shader.cpp
	src/texture.cpp
	src/camera.cpp
    src/framebuffer.cpp
)

target_precompile_headers(${PROJECT_NAME} PRIVATE pch.hpp)

add_subdirectory(dep/glad)
add_subdirectory(dep/stb_image)
add_subdirectory(dep/sdl3)

target_link_libraries(${PROJECT_NAME} 
	glad
	glm
	stb_image
	sdl3-static
)

target_include_directories(${PROJECT_NAME}
	PUBLIC dep/glad/include/
	PUBLIC dep/stb_image/include
	PUBLIC dep/sdl3/include
)
