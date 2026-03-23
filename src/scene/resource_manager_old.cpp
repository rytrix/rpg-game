#include "resource_manager.hpp"

#include "../renderer/model.hpp"
#include "../renderer/texture.hpp"

void SceneResources::init()
{
    m_model_cache.init();
    m_texture_cache.init();

    initialized = true;
}

SceneResources::~SceneResources()
{
    initialized = false;
}

Renderer::Model* SceneResources::get_model(ResourceHandle handle)
{
    util_assert(initialized == true, "Resources not initialized");
    return m_model_cache.get(handle);
}

Renderer::Texture* SceneResources::get_texture(ResourceHandle handle)
{
    util_assert(initialized == true, "Resources not initialized");
    return m_texture_cache.get(handle);
}
