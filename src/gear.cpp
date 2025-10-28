#include "gear.hpp"

Item::Item()
    :m_item_level(0)
{
}

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
    m_stats.m_stamina = base_stats.m_stamina * (ILVL_SCALING_FACTOR * m_item_level);
    m_stats.m_resource = base_stats.m_resource * (ILVL_SCALING_FACTOR * m_item_level);

    m_stats.m_armor = base_stats.m_armor * (ILVL_SCALING_FACTOR * m_item_level);
    m_stats.m_resist = base_stats.m_resist * (ILVL_SCALING_FACTOR * m_item_level);

    m_stats.m_primary = base_stats.m_primary * (ILVL_SCALING_FACTOR * m_item_level);
    m_stats.m_crit = base_stats.m_crit * (ILVL_SCALING_FACTOR * m_item_level);
    m_stats.m_haste = base_stats.m_haste * (ILVL_SCALING_FACTOR * m_item_level);
    m_stats.m_expertise = base_stats.m_expertise * (ILVL_SCALING_FACTOR * m_item_level);

    m_stats.m_spirit = base_stats.m_spirit * (ILVL_SCALING_FACTOR * m_item_level);
    m_stats.m_recovery = base_stats.m_recovery * (ILVL_SCALING_FACTOR * m_item_level);
}

