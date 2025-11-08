#pragma once

#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/fwd.hpp>

#include <glad/glad.h>
// #include <GLFW/glfw3.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>

#include <chrono>
#include <cstdlib>
#include <functional>
#include <print>

#define NODISCARD [[nodiscard]]
#define MAYBEUNUSED [[maybe_unused]]
#define UNLIKELY [[unlikely]]
#define UNUSED __attribute__((unused))

#define ARRAY_LEN(array) sizeof(array)/sizeof(array[0])
