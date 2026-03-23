#pragma once

#include "../utils/mapping.hpp"

using ResourceHandle = u32;

template <typename MapType, typename DataType, usize MaxResource>
class ResourceManager : public NoCopyNoMove {
public:
    ResourceManager() = default;
    ~ResourceManager();

    void init();

    template <typename... Args>
    ResourceHandle add(const MapType& id, Args&&... args);
    bool contains(const MapType& id);

    void remove(ResourceHandle handle);
    DataType* get(ResourceHandle handle);

private:
    bool initialized = false;
    DataType* m_dense_storage = nullptr;
    usize m_dense_storage_size = 0;

    Utils::Mapping<MapType, ResourceHandle> m_map;
};

template <typename MapType, typename DataType, usize MaxResources>
ResourceManager<MapType, DataType, MaxResources>::~ResourceManager()
{
    if (initialized) {
        for (auto pair : m_map) {
            util_assert(pair.second < MaxResources, std::format("ResourceManager handle {} >= MaxResources {}", pair.second, MaxResources));
            m_dense_storage[pair.second].~DataType();
        }

        free(m_dense_storage);
        initialized = false;
    }
}

template <typename MapType, typename DataType, usize MaxResources>
void ResourceManager<MapType, DataType, MaxResources>::init()
{
    m_dense_storage = (DataType*)malloc(sizeof(DataType) * MaxResources);
    util_assert(m_dense_storage != nullptr, "ResourceManager failed to malloc");
    initialized = true;
}

template <typename MapType, typename DataType, usize MaxResources>
template <typename... Args>
ResourceHandle ResourceManager<MapType, DataType, MaxResources>::add(const MapType& id, Args&&... args)
{
    util_assert(initialized == true, "ResourceManager has not been initialized");
    if (m_map.contains(id)) {
        return m_map.at(id);
    }

    ResourceHandle handle = m_map.map(id);

    util_assert(handle < MaxResources, std::format("ResourceManager handle {} >= MaxResources {}", handle, MaxResources));
    new (&m_dense_storage[handle]) DataType(std::forward<Args>(args)...);

    m_dense_storage_size++;

    return handle;
}

template <typename MapType, typename DataType, usize MaxResources>
bool ResourceManager<MapType, DataType, MaxResources>::contains(const MapType& id)
{
    util_assert(initialized == true, "ResourceManager has not been initialized");
    return m_map.contains(id);
}

template <typename MapType, typename DataType, usize MaxResources>
void ResourceManager<MapType, DataType, MaxResources>::remove(ResourceHandle handle)
{
    util_assert(initialized == true, "ResourceManager has not been initialized");
    util_assert(handle < MaxResources, std::format("ResourceManager handle {} >= MaxResources {}", handle, MaxResources));
    m_dense_storage[handle].~DataType();
    m_map.remove(handle);
}

template <typename MapType, typename DataType, usize MaxResources>
DataType* ResourceManager<MapType, DataType, MaxResources>::get(ResourceHandle handle)
{
    util_assert(initialized == true, "ResourceManager has not been initialized");
    util_assert(handle < MaxResources, std::format("ResourceManager handle {} >= MaxResources {}", handle, MaxResources));
    util_assert(handle != 0, "ResourceManager: attempting to get a handle of 0 (invalid)");
    return &m_dense_storage[handle];
}

#include "cache_types.hpp"

namespace Renderer {
class Model;
class Texture;
} // namespace Renderer

struct SceneResources : public NoCopyNoMove {
    SceneResources() = default;
    ~SceneResources();
    void init();

    ModelCache m_model_cache;
    TextureCache m_texture_cache;

    Renderer::Model* get_model(ResourceHandle handle);
    Renderer::Texture* get_texture(ResourceHandle handle);

private:
    bool initialized = false;
};
