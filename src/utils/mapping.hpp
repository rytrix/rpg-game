#pragma once

namespace Utils {

template <typename IDType = std::string, typename MapType = u32>
class Mapping : public NoCopyNoMove {
public:
    Mapping() = default;

    MapType map(const IDType& id);
    void remove(const IDType& id);
    void remove(const MapType& id);
    auto begin() { return m_map.begin(); }
    auto end() { return m_map.end(); }
    [[nodiscard]] MapType at(const IDType& id) const;
    [[nodiscard]] usize size() const;
    [[nodiscard]] bool contains(const IDType& id) const;

private:
    std::unordered_map<IDType, MapType> m_map;
    MapType m_size;
    MapType m_id_counter = 0;
    std::vector<MapType> m_free_ids;
};

template <typename IDType, typename MapType>
MapType Mapping<IDType, MapType>::map(const IDType& id)
{
    if (m_map.contains(id)) {
        return m_map.at(id);
    } else {
        if (m_free_ids.size() == 0) {
            m_map.insert({ id, m_id_counter });
            m_size++;
            return m_id_counter++;
        } else {
            MapType map_id = m_free_ids.back();
            m_free_ids.pop_back();
            m_map.insert({ id, map_id });
            m_size++;
            return map_id;
        }
    }
}

template <typename IDType, typename MapType>
void Mapping<IDType, MapType>::remove(const IDType& id)
{
    auto removed = m_map.erase(id);
    if (removed != m_map.end()) {
        MapType removed_id = removed;
        if (m_id_counter - 1 == removed_id) {
            m_id_counter--;
        } else {
            m_free_ids.push_back(removed_id);
        }
    }
}

template <typename IDType, typename MapType>
void Mapping<IDType, MapType>::remove(const MapType& id)
{
    for (auto& pair : m_map) {
        if (pair.second == id) {
            m_map.erase(pair.first);
            if (m_id_counter - 1 == id) {
                m_id_counter--;
            } else {
                m_free_ids.push_back(id);
            }

            break;
        }
    }
}

template <typename IDType, typename MapType>
MapType Mapping<IDType, MapType>::at(const IDType& id) const
{
    return m_map.at(id);
}

template <typename IDType, typename MapType>
usize Mapping<IDType, MapType>::size() const
{
    return m_size;
}

template <typename IDType, typename MapType>
[[nodiscard]] bool Mapping<IDType, MapType>::contains(const IDType& id) const
{
    return m_map.contains(id);
}

} // namespace Utils
