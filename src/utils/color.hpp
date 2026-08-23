#pragma once

namespace Utils {

namespace Color {

    constexpr u32 pack(u8 r, u8 g, u8 b, u8 a);
    constexpr u32 pack(glm::vec3 color);
    constexpr u32 pack(glm::vec4 color);

    constexpr glm::vec4 unpack(u8 r, u8 g, u8 b, u8 a);
    constexpr glm::vec4 unpack(u32 color);

    constexpr u32 pack(glm::vec3 color)
    {
        return pack(
            static_cast<u8>(color.r * 255.0),
            static_cast<u8>(color.g * 255.0),
            static_cast<u8>(color.b * 255.0),
            static_cast<u8>(255));
    }

    constexpr u32 pack(glm::vec4 color)
    {
        return pack(
            static_cast<u8>(color.r * 255.0),
            static_cast<u8>(color.g * 255.0),
            static_cast<u8>(color.b * 255.0),
            static_cast<u8>(color.a * 255.0));
    }

    constexpr u32 pack(u8 r, u8 g, u8 b, u8 a)
    {
        return ((u32)a << 24) | ((u32)b << 16) | ((u32)g << 8) | (u32)r;
    }

    constexpr glm::vec4 unpack(u8 r, u8 g, u8 b, u8 a)
    {
        return { static_cast<f32>(r / 255.0),
            static_cast<f32>(g / 255.0),
            static_cast<f32>(b / 255.0),
            static_cast<f32>(a / 255.0) };
    }

    constexpr glm::vec4 unpack(u32 color)
    {
        u8 r = (color >> 24) & UINT8_MAX;
        u8 g = (color >> 16) & UINT8_MAX;
        u8 b = (color >> 8) & UINT8_MAX;
        u8 a = color & UINT8_MAX;

        return unpack(r, g, b, a);
    }

    static constexpr glm::vec4 Red { 1.0, 0.0, 0.0, 1.0 };
    static constexpr glm::vec4 Green { 0.0, 1.0, 0.0, 1.0 };
    static constexpr glm::vec4 Blue { 0.0, 0.0, 1.0, 1.0 };

} // Namespace Color

} // Namespace Utils