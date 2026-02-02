#pragma once

namespace Utils {

template <typename T>
class Mapping : public NoCopyNoMove {
public:
    Mapping() = default;

    u64 map(const T& id);

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

} // namespace Utils