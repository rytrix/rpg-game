#pragma once

#include "../scene/resource_manager.hpp"

namespace Renderer {

struct DefaultTextures : public NoCopyNoMove {
    DefaultTextures() = default;
    explicit DefaultTextures(TextureCache* cache);
    ~DefaultTextures();
    void init(TextureCache* cache);
    [[nodiscard]] u32 get_albedo() const;
    [[nodiscard]] u32 get_metallic() const;
    [[nodiscard]] u32 get_normal() const;

private:
    bool initialized = false;
    u32 m_albedo = 0;
    u32 m_metallic = 0;
    u32 m_normal = 0;
    TextureCache* m_cache = nullptr;
};

} // namespace Renderer
