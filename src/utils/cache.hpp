#pragma once

namespace Utils {

template <typename ID, typename Type>
class Cache : public NoCopyNoMove {
public:
    Cache() = default;

    template <typename... Args>
    Type& get_or_create(ID id, Args&&... args);

private:
    std::vector<ID> m_ids;
    std::deque<Type> m_types;
};

template <typename ID, typename Type>
template <typename... Args>
Type& Cache<ID, Type>::get_or_create(ID id, Args&&... args)
{
    for (usize i = 0; i < m_ids.size(); i++) {
        if (id == m_ids.at(i)) {
            return m_types.at(i);
        }
    }

    m_ids.emplace_back(id);
    m_types.emplace_back(std::forward<Args>(args)...);
    return m_types.back();
}

} // namespace Utils