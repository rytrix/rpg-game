#pragma once

#include "mapping.hpp"

namespace Utils {

template <typename ID, typename Type>
class Cache : public NoCopyNoMove {
public:
    Cache() = default;

    template <typename... Args>
    Type& get_or_create(const ID& id, Args&&... args);

    [[nodiscard]] bool contains(const ID& id);

private:
    Mapping<ID, u64> m_mapping;
    std::deque<Type> m_types;
};

template <typename ID, typename Type>
template <typename... Args>
Type& Cache<ID, Type>::get_or_create(const ID& id, Args&&... args)
{
    u64 map_id = m_mapping.map(id);
    if (map_id >= m_types.size()) {
        m_types.emplace_back(std::forward<Args>(args)...);
    }
    return m_types.at(map_id);
}

template <typename ID, typename Type>
[[nodiscard]] bool Cache<ID, Type>::contains(const ID& id)
{
    return m_mapping.contains(id);
}

} // namespace Utils
