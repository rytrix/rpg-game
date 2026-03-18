#pragma once

#include "../utils/mapping.hpp"

using ResourceHandle = u32;

template <typename MapType, typename DataType, usize MaxResources>
class ResourceManager : public NoCopyNoMove {
public:
    ResourceManager() = default;
    ~ResourceManager();

    void init();

    template <typename... Args>
    ResourceHandle add(const MapType& id, Args&&... args);
    void remove(ResourceHandle handle);

    DataType* get(ResourceHandle handle);

private:
    bool initialized = false; // TODO
    DataType* m_dense_storage = nullptr;
    usize m_dense_storage_size = 0;

    Utils::Mapping<MapType, ResourceHandle> m_map;
};

template <typename MapType, typename DataType, usize MaxResources>
ResourceManager<MapType, DataType, MaxResources>::~ResourceManager()
{
    for (auto pair : m_map) {
        m_dense_storage[pair.second].~DataType();
    }

    free(m_dense_storage);
}

template <typename MapType, typename DataType, usize MaxResources>
void ResourceManager<MapType, DataType, MaxResources>::init()
{
    m_dense_storage = (DataType*)malloc(sizeof(DataType) * MaxResources);
}

template <typename MapType, typename DataType, usize MaxResources>
template <typename... Args>
ResourceHandle ResourceManager<MapType, DataType, MaxResources>::add(const MapType& id, Args&&... args)
{
    if (m_map.contains(id)) {
        return m_map.at(id);
    }

    ResourceHandle handle = m_map.map(id);

    new (&m_dense_storage[handle]) DataType(std::forward<Args...>(args...));

    m_dense_storage_size++;

    return handle;
}

template <typename MapType, typename DataType, usize MaxResources>
void ResourceManager<MapType, DataType, MaxResources>::remove(ResourceHandle handle)
{
    m_dense_storage[handle].~DataType();
    m_map.remove(handle);
}

template <typename MapType, typename DataType, usize MaxResources>
DataType* ResourceManager<MapType, DataType, MaxResources>::get(ResourceHandle handle)
{
    return &m_dense_storage[handle];
}
