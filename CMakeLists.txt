cmake_minimum_required(VERSION 3.22.0)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(_DEBUG)
endif()

project(rpg)
add_executable(${PROJECT_NAME} 
    src/main.cpp 
    src/app.cpp 

    src/physics/engine.cpp
    
	src/utils/deltatime.cpp

    src/game_logic/gear.cpp 
    src/game_logic/character.cpp

	src/renderer/window.cpp
	src/renderer/buffer.cpp
	src/renderer/vertex.cpp
	src/renderer/shader.cpp
	src/renderer/texture.cpp
	src/renderer/camera.cpp
    src/renderer/framebuffer.cpp
    src/renderer/renderbuffer.cpp
	src/renderer/mesh.cpp
	src/renderer/model.cpp
    src/renderer/gbuffer.cpp
    src/renderer/shadowmap.cpp
    src/renderer/quad.cpp
    src/renderer/light/point.cpp
    src/renderer/light/directional.cpp
)

target_precompile_headers(${PROJECT_NAME} PUBLIC src/pch.hpp)

add_subdirectory(dep/sqlite)
add_subdirectory(dep/glad)
add_subdirectory(dep/stb)

set(SDL_SHARED OFF CACHE BOOL "Prefer building static" FORCE)
set(SDL_STATIC ON CACHE BOOL "Prefer building static" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "Not testing SDL3")
add_subdirectory(dep/sdl3)

set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "Enable compiling with extra warnings" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "Disable building assimp tests" FORCE)
add_subdirectory(dep/assimp)

set(ENABLE_ALL_WARNINGS OFF CACHE BOOL "Stop warnings as errors for JoltPhysics" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libs for JoltPhysics" FORCE)
add_subdirectory(dep/JoltPhysics/Build)

add_library(imgui 
    dep/imgui/imgui.cpp
    dep/imgui/imgui_demo.cpp
    dep/imgui/imgui_draw.cpp
    dep/imgui/imgui_tables.cpp
    dep/imgui/imgui_widgets.cpp

    dep/imgui/backends/imgui_impl_sdl3.cpp
    dep/imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui
    PUBLIC dep/imgui
    PUBLIC dep/imgui/backends
)

target_link_libraries(imgui
    GL
    SDL3::SDL3
)

target_link_libraries(${PROJECT_NAME} 
    sqlite 
	glad
	stb_image
	SDL3::SDL3
    assimp
	Jolt::Jolt
    imgui
)

target_include_directories(${PROJECT_NAME} 
    PUBLIC src/renderer/include
    PUBLIC dep
	PUBLIC dep/glad/include/
	PUBLIC dep/stb/include
	PUBLIC dep/sdl3/include
	PUBLIC dep/glm/
    PUBLIC dep/assimp/include
    PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/dep/assimp/include
    PUBLIC dep/JoltPhysics
)
