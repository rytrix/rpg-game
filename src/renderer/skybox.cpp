#include "skybox.hpp"

#include <cstddef>

#include "../scene/shader_preprocessor.hpp"
#include "../utils/file.hpp"

namespace Renderer {

Skybox::Skybox(SkyboxInfo& info)
{
    init(info);
}

void Skybox::init(SkyboxInfo& info)
{
    util_assert(initialized == false, "already initialized");
    m_vao.init();

    TextureInfo texture_info;
    texture_info.dimensions = GL_TEXTURE_CUBE_MAP;
    texture_info.size = { .width = 512, .height = 512, .depth = 0 };
    texture_info.internal_format = GL_RGB8;
    texture_info.wrap_s = GL_CLAMP_TO_BORDER;
    texture_info.wrap_t = GL_CLAMP_TO_BORDER;
    texture_info.wrap_r = GL_CLAMP_TO_BORDER;
    texture_info.min_filter = GL_NEAREST;
    texture_info.mag_filter = GL_NEAREST;
    m_cubemap.init(texture_info);

    TextureSubimageInfo subimage_info;
    subimage_info.format = GL_RGB;
    subimage_info.level = 0;
    subimage_info.type = GL_UNSIGNED_BYTE;

    TextureSize size;
    int nr_channels;
    stbi_set_flip_vertically_on_load(0);
    const char* file = info.file;
    unsigned char* data = stbi_load(file, &size.width, &size.height, &nr_channels, 3);
    if (data == nullptr) {
        util_error(std::format("failed to load texture {}", file));
    }
    util_assert(nr_channels == 3, "expected 3 channels");

    const u32 atlas_width = size.width;
    const u32 face_size = size.width / 4;
    LOG_INFO(std::format("Skybox atlas width: \"{}\", face size \"{}\"", atlas_width, face_size));

    // GL_TEXTURE_CUBE_MAP_POSITIVE_X (Right)	0
    // GL_TEXTURE_CUBE_MAP_NEGATIVE_X (Left)	1
    // GL_TEXTURE_CUBE_MAP_POSITIVE_Y (Top)	    2
    // GL_TEXTURE_CUBE_MAP_NEGATIVE_Y (Bottom)	3
    // GL_TEXTURE_CUBE_MAP_POSITIVE_Z (Back)	4
    // GL_TEXTURE_CUBE_MAP_NEGATIVE_Z (Front)	5
    glm::ivec2 texture_offsets[] = {
        { face_size * 2, face_size * 1 },
        { face_size * 0, face_size * 1 },
        { face_size * 1, face_size * 0 },
        { face_size * 1, face_size * 2 },
        { face_size * 1, face_size * 1 },
        { face_size * 3, face_size * 1 },
    };

    char* const image_sub_data = (char*)malloc(512 * 512 * 3);

    for (u32 i = 0; i < 6; i++) {
        int face_start_x = texture_offsets[i].x;
        int face_start_y = texture_offsets[i].y;

        for (u32 j = 0; j < face_size; j++) {
            int current_atlas_y = face_start_y + j;
            char* dest = image_sub_data + (j * face_size * nr_channels);
            unsigned char* src = data + (face_start_x + (current_atlas_y * atlas_width)) * nr_channels;
            memcpy(dest, src, face_size * nr_channels);
        }

        subimage_info.offsets = { .width = 0, .height = 0, .depth = (GLint)i }; // depth is cube map face
        subimage_info.size = { .width = 512, .height = 512, .depth = 1 };
        subimage_info.pixels = image_sub_data;
        m_cubemap.sub_image(subimage_info);
    }

    free(image_sub_data);

    stbi_image_free(data);

    setup_shader();
    initialized = true;
}

void Skybox::draw(const Camera& camera)
{
    util_assert(initialized == true, "not initialized");
    m_vao.bind();

    m_shader.bind();
    glDepthFunc(GL_LEQUAL);

    m_shader.set_mat4("proj", camera.get_proj());
    auto view = glm::mat4(glm::mat3(camera.get_view()));
    m_shader.set_mat4("view", view);

    auto texture_unit = Texture::get_texture_unit();
    m_cubemap.bind(texture_unit);
    m_shader.set_int("skybox", (int)texture_unit);

    glDrawArrays(GL_TRIANGLES, 0, 36);

    glDepthFunc(GL_LESS);
    Texture::drop_texture_units(1);
}

void Skybox::setup_shader()
{
    ShaderInfoData<2> out;

    std::vector<char> text_shader_file = read_file<char>("res/shaders/skybox/skybox.glsl");
    std::string_view text_shader_file_view = { text_shader_file.data(), text_shader_file.size() };

    // Vertex Shader
    out.data.at(0) = "#version 460 core\n";
    out.data.at(0) += get_lines_between_delims(text_shader_file_view, "// Vertex Begin", "// Vertex End");

    // Fragment Shader
    out.data.at(1) += "#version 460 core\n";
    out.data.at(1) += get_lines_between_delims(text_shader_file_view, "// Fragment Begin", "// Fragment End");

    out.populate_info();

    m_shader.init(out.info.data(), out.info.size());
}

} // namespace Renderer
