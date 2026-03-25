#pragma once

#include "../scene/resource_manager2.hpp"

namespace Renderer {

class ShaderProgram;

struct RandomSamplingTexture : public NoCopyNoMove {
    static RandomSamplingTexture create(i32 window_size, i32 filter_size, i32 radius, TextureCache* cache);

    [[nodiscard]] Handle get_handle() const { return m_handle; }
    [[nodiscard]] i32 get_window_size() const { return m_window_size; }
    [[nodiscard]] i32 get_filter_size() const { return m_filter_size; }

    void bind_uniforms(ShaderProgram& shader, const char* uniform_name, TextureCache* cache);

    i32 m_radius;

private:
    RandomSamplingTexture(Handle handle, i32 window_size, i32 filter_size, i32 radius);
    static Handle create_random_sampling_texture(i32 window_size, i32 filter_size, TextureCache* cache);

    Handle m_handle;
    i32 m_window_size;
    i32 m_filter_size;
};

} // namespace Renderer
