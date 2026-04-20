#pragma once

#include "string.hpp"
#include <expected>

constexpr const char* opengl_error_to_string(GLenum error)
{
    const char* error_string = nullptr;
    switch (error) {
        case GL_INVALID_ENUM:
            error_string = "GL_INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            error_string = "GL_INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            error_string = "GL_INVALID_OPERATION";
            break;
        case GL_OUT_OF_MEMORY:
            error_string = "GL_OUT_OF_MEMORY";
            break;
        default:
            error_string = "UNKNOWN";
    }

    return error_string;
}

struct OpenGLError {
    GLenum error;
    const char* opengl_function_name;
    std::source_location location;

    Utils::StaticString<512> string()
    {
        const char* error_string = opengl_error_to_string(error);

        Utils::StaticString<512> result;
        result.format("OpenGL Error [{}] from {} {}:{} {}",
            opengl_function_name,
            error_string,
            location.file_name(),
            location.line(),
            location.function_name());

        return result;
    }
};

constexpr std::expected<void, OpenGLError> check_opengl_error(const char* opengl_function_name, std::source_location location = std::source_location::current())
{
    GLenum error = glGetError();
    if (error != 0) {
        return std::unexpected<OpenGLError> {
            {
                .error = error,
                .opengl_function_name = opengl_function_name,
                .location = location,
            }
        };
    }

    return {};
}

#define GL_CHECK(opengl_function_call)                   \
    [&]() {                                              \
        opengl_function_call;                            \
        return check_opengl_error(#opengl_function_call) \
    }();