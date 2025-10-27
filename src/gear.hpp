#pragma once

template <typename T>
struct Statsheet {
    T m_stamina = {};
    T m_resource = {};

    T m_armor = {};
    T m_resist = {};

    T m_primary = {};
    T m_crit = {};
    T m_haste = {};
    T m_expertise = {};
    T m_spirit = {};
};

class Item {
public:
    Item(u32 item_level, Statsheet<u64> base_stats);

    const Statsheet<u64>& get_statsheet();

private:
    u32 m_item_level;
    Statsheet<u64> m_stats;

    static constexpr u64 SCALING_FACTOR = 1.0;

    void calculate_leveled_stats(Statsheet<u64> base_stats);
};
