#include "text.hpp"
#include "../scene/shader_preprocessor.hpp"
#include "../utils/file.hpp"

namespace Renderer {

namespace {

    struct TextVertex {
        glm::vec4 pos_uv;
    };

} // anonymous namespace

TextRenderer::TextRenderer(const char* font_path, u32 font_height)
{
    init(font_path, font_height);
}

void TextRenderer::init(const char* font_path, u32 font_height)
{
    if (FT_Init_FreeType(&m_freetype)) {
        util_error("Failed to init freetype");
    }

    if (FT_New_Face(m_freetype, font_path, 0, &m_face)) {
        util_error(std::format("Failed to load font \"{}\"", font_path));
    }

    FT_Set_Pixel_Sizes(m_face, 0, font_height);
    m_font_height = font_height;

    update_view(800.0F, 800.0F);

    setup_characters();
    setup_quad();
    setup_shader();
}

TextRenderer::~TextRenderer()
{
    FT_Done_Face(m_face);
    FT_Done_FreeType(m_freetype);
}

void TextRenderer::draw_text(u32 x, u32 y, const char* text, glm::vec3 color, float scale)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDisable(GL_DEPTH_TEST);

    m_vao.bind();

    m_shader.bind();
    auto texture_unit = Texture::get_texture_unit();
    m_texture_atlas.bind(texture_unit);

    m_shader.set_int("text_atlas", (int)texture_unit);
    m_shader.set_vec3("text_color", color);
    m_shader.set_mat4("projection", m_projection);

    usize text_length = strlen(text);
    std::vector<TextVertex> vertices;
    vertices.reserve(text_length * 6);
    for (u32 i = 0; i < text_length; i++) {
        Character character = m_characters[text[i]];
        float x_pos = x + ((float)character.bearing.x * scale);

        float y_pos = y - (((float)character.size.y - (float)character.bearing.y) * scale);

        float w = (float)character.size.x * scale;
        float h = (float)character.size.y * scale;

        float uv_x = character.uv_offset.x;
        float uv_x_max = uv_x + character.uv_size.x;

        float uv_y = character.uv_offset.y;
        float uv_y_max = uv_y + character.uv_size.y;

        TextVertex vertex {};
        vertex.pos_uv = { x_pos, y_pos + h, uv_x, uv_y };
        vertices.emplace_back(vertex);

        vertex.pos_uv = { x_pos, y_pos, uv_x, uv_y_max };
        vertices.emplace_back(vertex);

        vertex.pos_uv = { x_pos + w, y_pos, uv_x_max, uv_y_max };
        vertices.emplace_back(vertex);

        vertex.pos_uv = { x_pos, y_pos + h, uv_x, uv_y };
        vertices.emplace_back(vertex);

        vertex.pos_uv = { x_pos + w, y_pos, uv_x_max, uv_y_max };
        vertices.emplace_back(vertex);

        vertex.pos_uv = { x_pos + w, y_pos + h, uv_x_max, uv_y };
        vertices.emplace_back(vertex);

        x += (character.advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
    }

    usize size = vertices.size() * sizeof(TextVertex);
    if (size > m_ssbo.get_buffer_size()) {
        m_ssbo.deinit();
        m_ssbo.init(3, size);
    }

    void* ptr = m_ssbo.get_ptr();
    memcpy(ptr, vertices.data(), size);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_ssbo.get_id());
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)vertices.size());

    Texture::drop_texture_units(1);
    m_ssbo.increment_frame();
}

void TextRenderer::update_view(float width, float height)
{
    m_projection = glm::ortho(0.0F, width, 0.0F, height);
}

void TextRenderer::setup_characters()
{
    m_pixel_height = (m_face->size->metrics.ascender - m_face->size->metrics.descender) >> 6;

    Renderer::TextureInfo info;
    info.min_filter = GL_LINEAR;
    info.mag_filter = GL_LINEAR;
    info.wrap_s = GL_CLAMP_TO_EDGE;
    info.wrap_t = GL_CLAMP_TO_EDGE;
    info.wrap_r = GL_CLAMP_TO_EDGE;
    info.internal_format = GL_R8;
    info.size.width = ATLAS_WIDTH;
    info.size.height = (GLint)m_pixel_height;
    info.size.depth = 1;
    m_texture_atlas.init(info);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction

    for (unsigned char c = 0; c < 128; c++) {
        if (FT_Load_Char(m_face, c, FT_LOAD_RENDER)) {
            LOG_ERROR(std::format("FREETYPE: Failed to load glyph {}", (char)c));
            continue;
        }

        TextureSubimageInfo subimage_info {};
        subimage_info.offsets.width = (GLint)m_current_atlas_width;
        subimage_info.offsets.height = 0;
        subimage_info.size.width = (GLint)m_face->glyph->bitmap.width;
        subimage_info.size.height = (GLint)m_face->glyph->bitmap.rows;
        subimage_info.format = GL_RED;
        subimage_info.type = GL_UNSIGNED_BYTE;
        subimage_info.pixels = m_face->glyph->bitmap.buffer;
        m_texture_atlas.sub_image(subimage_info);

        Character character {};
        character.uv_offset.x = (float)subimage_info.offsets.width / (float)ATLAS_WIDTH;
        character.uv_offset.y = (float)subimage_info.offsets.height / (float)m_pixel_height;
        character.uv_size.x = (float)subimage_info.size.width / (float)ATLAS_WIDTH;
        character.uv_size.y = (float)subimage_info.size.height / (float)m_pixel_height;
        character.size = { m_face->glyph->bitmap.width, m_face->glyph->bitmap.rows };
        character.bearing = { m_face->glyph->bitmap_left, m_face->glyph->bitmap_top };
        character.advance = m_face->glyph->advance.x;
        m_characters.insert({ c, character });

        m_current_atlas_width += m_face->glyph->bitmap.width + 1;
    }
}

void TextRenderer::setup_quad()
{
    m_vao.init();
    m_ssbo.init(3, 1);

    m_vao.bind();
}

void TextRenderer::setup_shader()
{
    ShaderInfoData<2> out;

    std::vector<char> pbr_file = read_file<char>("res/shaders/text_rendering/text_combined.glsl");
    std::string_view pbr_file_view = { pbr_file.data(), pbr_file.size() };

    // Vertex Shader
    out.data.at(0) = std::format("#version 460 core\n");
    out.data.at(0) += get_lines_between_delims(pbr_file_view, "// Vertex Begin", "// Vertex End");

    // Fragment Shader
    out.data.at(1) += "#version 460 core\n";
    out.data.at(1) += get_lines_between_delims(pbr_file_view, "// Fragment Begin", "// Fragment End");

    out.populate_info();

    m_shader.init(out.info.data(), out.info.size());
}

} // namespace Renderer
