#include "texture.hpp"

namespace Renderer {

Texture::Texture()
{
}

Texture::Texture(TextureInfo* info)
{
    init(info);
}

Texture::~Texture()
{
    glDeleteTextures(1, &id);
}

void Texture::init(TextureInfo* info)
{
    dimensions = info->dimensions;
    glCreateTextures(dimensions, 1, &id);

    glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, info->min_filter);
    glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, info->mag_filter);
    glTextureParameteri(id, GL_TEXTURE_WRAP_S, info->wrap_s);
    glTextureParameteri(id, GL_TEXTURE_WRAP_T, info->wrap_t);

    if (info->fromFile) {
        fromFile(info->file);
    } else {
        textureStorage(&info->size, info->internalFormat);
    }

    if (info->mipmaps) {
        generateMipmap();
    }
}

void Texture::generateMipmap()
{
    glGenerateTextureMipmap(id);
}

void Texture::subImage(TextureSubimageInfo* info)
{
    switch (dimensions) {
    case GL_TEXTURE_1D:
        glTextureSubImage1D(id,
            info->level,
            info->offsets.width,
            info->size.width,
            info->format,
            info->type,
            info->pixels);
        break;
    case GL_TEXTURE_2D:
        glTextureSubImage2D(id,
            info->level,
            info->offsets.width,
            info->offsets.height,
            info->size.width,
            info->size.height,
            info->format,
            info->type,
            info->pixels);
        break;
    case GL_TEXTURE_3D:
        glTextureSubImage3D(id,
            info->level,
            info->offsets.width,
            info->offsets.height,
            info->offsets.depth,
            info->size.width,
            info->size.height,
            info->size.depth,
            info->format,
            info->type,
            info->pixels);
        break;
    default:
        std::print("invalid texture dimensions {}\n", dimensions);
        exit(EXIT_FAILURE);
    }
}

void Texture::bind(GLuint textureUnit)
{
    glBindTextureUnit(textureUnit, id);
}

void Texture::textureStorage(TextureSize* size, GLenum internalFormat)
{
    switch (dimensions) {
    case GL_TEXTURE_1D:
        glTextureStorage1D(id, 1, internalFormat, size->width);
        break;
    case GL_TEXTURE_2D:
        glTextureStorage2D(id, 1, internalFormat, size->width, size->height);
        break;
    case GL_TEXTURE_3D:
        glTextureStorage3D(id, 1, internalFormat, size->width, size->height, size->depth);
        break;
    default:
        std::print("invalid texture dimensions {}\n", dimensions);
        exit(EXIT_FAILURE);
    }
}

void Texture::fromFile(const char* file)
{
    if (dimensions != GL_TEXTURE_2D) {
        std::print("currently only 2D textures are supported to be loaded from files\n");
        exit(EXIT_FAILURE);
    }

    stbi_set_flip_vertically_on_load(true);
    TextureSize size;
    int nrChannels;
    unsigned char* data = stbi_load(file, &size.width, &size.height, &nrChannels, 0);
    if (data == nullptr) {
        std::print("failed to load texture {}", file);
        exit(EXIT_FAILURE);
    }

    textureStorage(&size, GL_RGB8);

    TextureSubimageInfo info {};
    info.format = GL_RGB;
    info.type = GL_UNSIGNED_BYTE;
    info.size = size;
    info.pixels = data;

    subImage(&info);

    stbi_image_free(data);
}

} // namespace Mcq
