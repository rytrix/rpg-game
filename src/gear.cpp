#include "gear.hpp"

Item::Item(u32 item_level, Statsheet<u64> base_stats)
    : m_item_level(item_level)
{
    // if (base_stats.m_resource != 0) {
    //     throw std::runtime_error("Item resource stat is greater than zero");
    // }
    calculate_leveled_stats(base_stats);
}

const Statsheet<u64>& Item::get_statsheet()
{
    return m_stats;
}

void Item::calculate_leveled_stats(Statsheet<u64> base_stats)
{
    m_stats.m_stamina = base_stats.m_stamina * (SCALING_FACTOR * m_item_level);
    m_stats.m_resource = base_stats.m_resource * (SCALING_FACTOR * m_item_level);

    m_stats.m_armor = base_stats.m_armor * (SCALING_FACTOR * m_item_level);
    m_stats.m_resist = base_stats.m_resist * (SCALING_FACTOR * m_item_level);

    m_stats.m_primary = base_stats.m_primary * (SCALING_FACTOR * m_item_level);
    m_stats.m_crit = base_stats.m_crit * (SCALING_FACTOR * m_item_level);
    m_stats.m_haste = base_stats.m_haste * (SCALING_FACTOR * m_item_level);
    m_stats.m_expertise = base_stats.m_expertise * (SCALING_FACTOR * m_item_level);
    m_stats.m_spirit = base_stats.m_spirit * (SCALING_FACTOR * m_item_level);
}

