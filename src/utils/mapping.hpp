#pragma once

namespace Utils {

template <typename T>
class Mapping : public NoCopyNoMove {
public:
    Mapping() = default;

    u64 map(const T& id);
    [[nodiscard]] u64 at(const T& id) const;
    [[nodiscard]] usize size() const;
    [[nodiscard]] bool contains(const T& id) const;

private:
    std::unordered_map<T, u64> m_map;
    u64 m_id_counter = 0;
};

template <typename T>
u64 Mapping<T>::map(const T& id)
{
    if (m_map.contains(id)) {
        return m_map.at(id);
    } else {
        m_map.insert({ id, m_id_counter });
        return m_id_counter++;
    }
}

template <typename T>
u64 Mapping<T>::at(const T& id) const
{
    return m_map.at(id);
}

template <typename T>
usize Mapping<T>::size() const
{
    return m_id_counter;
}

template <typename T>
[[nodiscard]] bool Mapping<T>::contains(const T& id) const
{
    return m_map.contains(id);
}

} // namespace Utils