#pragma once

#include "../scene/resource_manager2.hpp"

namespace Renderer {

struct DefaultTextures : public NoCopyNoMove {
    DefaultTextures() = default;
    explicit DefaultTextures(TextureCache* cache);
    ~DefaultTextures();
    void init(TextureCache* cache);

    [[nodiscard]] Handle get_albedo() const;
    [[nodiscard]] Handle get_metallic() const;
    [[nodiscard]] Handle get_normal() const;

private:
    bool initialized = false;
    Handle m_albedo;
    Handle m_metallic;
    Handle m_normal;

    TextureCache* m_cache = nullptr;
};

} // namespace Renderer
