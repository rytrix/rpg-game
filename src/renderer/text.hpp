#pragma once

#include "buffer.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "vertex.hpp"

namespace Renderer {

class TextRenderer : public NoCopyNoMove {
public:
    TextRenderer() = default;
    TextRenderer(const char* font_path, u32 font_height);
    ~TextRenderer();

    void init(const char* font_path, u32 font_height);

    void draw_text(u32 x, u32 y, const char* text, glm::vec3 color, float scale = 1.0F);

    void update_view(float width, float height);

    u32 get_max_pixel_height() const { return m_pixel_height; };

private:
    FT_Library m_freetype;
    FT_Face m_face;

    struct Character {
        bool valid = false;
        // Texture Atlas Coordinates
        glm::vec2 uv_offset;
        glm::vec2 uv_size;

        // Freetype Info
        glm::ivec2 size;
        glm::ivec2 bearing;
        u32 advance;
    };

    std::vector<Character> m_characters;

    u32 m_font_height;
    u32 m_pixel_height;
    glm::mat4 m_projection;
    static constexpr u32 ATLAS_WIDTH = 4000;
    u32 m_current_atlas_width = 0;
    Texture m_texture_atlas;

    struct GlyphInstance {
        float x_pos, y_pos, w, h;
        float uv_x, uv_y, uv_x_max, uv_y_max;
    };
    std::vector<GlyphInstance> m_vertices;

    VertexArray m_vao;
    MappedBuffer m_ssbo;
    Shader m_shader;

    void setup_atlas();
    int load_glyph(char c);
    void setup_quad();
    void setup_shader();
};

} // namespace Renderer
