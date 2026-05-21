#pragma once

#include "../renderer/shader.hpp"

std::string get_lines_between_delims_inclusive(std::string_view string, std::string_view start, std::string_view end);
std::string get_lines_between_delims(std::string_view string, std::string_view start, std::string_view end);

template <usize S>
struct ShaderInfoData {
    std::array<std::string, S> data;
    std::array<Renderer::ShaderInfo, S> info;

    void populate_info()
    {
        if constexpr (S == 2) {
            info.at(0) = {
                .is_file = false,
                .shader = data.at(0).c_str(),
                .type = GL_VERTEX_SHADER,
            };
            info.at(1) = {
                .is_file = false,
                .shader = data.at(1).c_str(),
                .type = GL_FRAGMENT_SHADER,
            };
        } else if constexpr (S == 3) {
            info.at(0) = {
                .is_file = false,
                .shader = data.at(0).c_str(),
                .type = GL_VERTEX_SHADER,
            };
            info.at(1) = {
                .is_file = false,
                .shader = data.at(1).c_str(),
                .type = GL_GEOMETRY_SHADER,
            };
            info.at(2) = {
                .is_file = false,
                .shader = data.at(2).c_str(),
                .type = GL_FRAGMENT_SHADER,
            };
        }
    }
};
