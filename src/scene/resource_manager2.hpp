#pragma once

struct Generation {
    u32 valid : 1;
    u32 generation : 31;
};

struct Handle {
    Generation generation;
    u32 index;
};

template <typename DataType>
class ResourceManager2 : public NoCopyNoMove {
public:
    void init(usize size);
    ~ResourceManager2();

    Handle new_handle();
    void remove_handle(Handle handle);
    DataType* get(Handle handle);
    template <typename... Args>
    DataType* create(Handle handle, Args&&... args);

private:
    static constexpr u32 INVALID_NEXT_FREE = UINT32_MAX;
    struct InternalData {
        Generation generation = { .valid = 0, .generation = 0 };
        union {
            DataType data; // Valid == 1
            u32 next_free; // Valid == 0
        };
    };

    InternalData* m_data = nullptr;
    usize m_data_size = 0;
    usize m_data_capacity = 0;
    usize m_next_free_index = INVALID_NEXT_FREE;
};

template <typename DataType>
ResourceManager2<DataType>::~ResourceManager2()
{
    for (u32 i = 0; i < m_data_size; i++) {
        if (m_data[i].generation.valid == 1) {
            m_data[i].data.~DataType();
        }
    }

    delete[] m_data;
}

template <typename DataType>
void ResourceManager2<DataType>::init(usize size)
{
    util_assert(size < INVALID_NEXT_FREE, std::format("ResourceManager2 size {} greater than UINT32_MAX", size));
    m_data = new InternalData[size]();
    m_data_capacity = size;
}

template <typename DataType>
Handle ResourceManager2<DataType>::new_handle()
{
    util_assert(m_data_size < m_data_capacity,
        std::format("ResourceManager2 data_size {} >= data_capacity {}", m_data_size, m_data_capacity));

    u32 index;
    if (m_next_free_index != INVALID_NEXT_FREE) {
        u32 prev_free_index = m_next_free_index;
        u32 free_index = m_data[prev_free_index].next_free;
        util_assert(m_data[prev_free_index].generation.valid == 0, "invalid generation while searching for free node");
        if (free_index == INVALID_NEXT_FREE) {
            m_next_free_index = INVALID_NEXT_FREE;
            index = prev_free_index;
        } else {
            while (m_data[free_index].next_free != INVALID_NEXT_FREE) {
                prev_free_index = free_index;
                free_index = m_data[free_index].next_free;
                util_assert(m_data[prev_free_index].generation.valid == 0, "invalid generation while searching for free node");
            }
            m_data[prev_free_index].next_free = INVALID_NEXT_FREE;
            index = free_index;
        }
    } else {
        index = m_data_size++;
    }

    m_data[index].generation.valid = 1;

    Handle handle;
    handle.generation = m_data[index].generation;
    handle.index = index;

    return handle;
}

template <typename DataType>
void ResourceManager2<DataType>::remove_handle(Handle handle)
{
    util_assert(handle.generation.valid == 1, "ResourceManager2 attempting to remove invalid handle");
    util_assert(handle.index < m_data_size, std::format("ResourceManager2 handle index {} >= data_size {}", handle.index, m_data_size));

    m_data[handle.index].generation.valid = 0;
    m_data[handle.index].generation.generation++;
    // TODO: check if DataType has ~DataType?
    m_data->data.~DataType();
    m_data[handle.index].next_free = INVALID_NEXT_FREE;

    if (handle.index == m_data_size - 1) {
        m_data_size--;
        return;
    }

    if (m_next_free_index == INVALID_NEXT_FREE) {
        m_next_free_index = handle.index;
        return;
    }

    u32 prev_free_index = m_next_free_index;
    u32 free_index = m_data[prev_free_index].next_free;
    util_assert(m_data[prev_free_index].generation.valid == 0, "invalid generation while searching for free node");
    while (free_index != INVALID_NEXT_FREE) {
        prev_free_index = free_index;
        free_index = m_data[prev_free_index].next_free;
        util_assert(m_data[prev_free_index].generation.valid == 0, "invalid generation while searching for free node");
    }

    m_data[prev_free_index].next_free = handle.index;
}

template <typename DataType>
DataType* ResourceManager2<DataType>::get(Handle handle)
{
    util_assert(handle.index < m_data_capacity, std::format("ResourceManager2 handle index {} >= data_capacity {}", handle.index, m_data_size));
    util_assert(handle.generation.valid == 1, "ResourceManager2 attempting to remove invalid handle");

    if (handle.index >= m_data_size) {
        LOG_WARN(std::format("ResourceManager2 data at handle index {} has been invalidated", handle.index));
        return nullptr;
    }

    InternalData& data = m_data[handle.index];
    if (data.generation.valid != 1) {
        LOG_WARN(std::format("ResourceManager2 data at handle index {} is invalid", handle.index));
        return nullptr;
    }

    if (data.generation.generation != handle.generation.generation) {
        LOG_WARN(std::format("ResourceManager2 data at handle index {} has been invalidated", handle.index));
        return nullptr;
    }

    return &data.data;
}

template <typename DataType>
template <typename... Args>
DataType* ResourceManager2<DataType>::create(Handle handle, Args&&... args)
{
    DataType* data = get(handle);
    if (data == nullptr) {
        return data;
    } else {
        new (data) DataType(std::forward<Args>(args)...);
    }
}

void run_fuzzer(size_t iterations, size_t pool_size);
