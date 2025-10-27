#include "character.hpp"

Ability::Ability(Statsheet<f64> scaling_power)
    : m_scaling_power(scaling_power)
{
}

[[nodiscard]] f64 Ability::get_effectiveness(const Statsheet<u64>& max_stats) const
{
    f64 effectiveness = m_scaling_power.m_primary * static_cast<f64>(max_stats.m_primary);

    effectiveness += m_scaling_power.m_haste * static_cast<f64>(max_stats.m_haste);

    effectiveness += m_scaling_power.m_expertise * static_cast<f64>(max_stats.m_expertise);

    effectiveness += m_scaling_power.m_spirit * static_cast<f64>(max_stats.m_spirit);

    effectiveness = crit_effectiveness(effectiveness, m_scaling_power.m_crit, static_cast<f64>(max_stats.m_crit));

    return effectiveness;
}

[[nodiscard]] u64 Ability::get_cost(const Statsheet<u64>& max_stats) const
{
    return static_cast<u64>(static_cast<f64>(max_stats.m_resource) * m_scaling_power.m_resource);
}

[[nodiscard]] f64 Ability::crit_effectiveness(f64 effectiveness, f64 scaling_power, f64 crit_stat)
{
    f64 crit_chance = scaling_power * crit_stat;
    f64 overflow_crit = 0.0;

    if (crit_chance >= CRIT_CAP) {
        overflow_crit = crit_chance - CRIT_CAP;
        crit_chance = CRIT_CAP;
    }

    static thread_local std::mt19937 generator(std::random_device {}());
    std::uniform_int_distribution<int> distribution(0, 100);
    int value = distribution(generator);

    if (crit_chance <= value) {
        return effectiveness * (2.0 + overflow_crit);
    } else {
        return effectiveness;
    }
}

void Character::calculate_max_stats()
{
    Statsheet<u64> stats = {};

    for (auto& item : m_items) {
        const auto item_stats = item.get_statsheet();

        stats.m_stamina += item_stats.m_stamina;
        stats.m_resource += item_stats.m_resource;

        stats.m_armor += item_stats.m_armor;
        stats.m_resist += item_stats.m_resist;

        stats.m_primary += item_stats.m_primary;
        stats.m_crit += item_stats.m_crit;
        stats.m_haste += item_stats.m_haste;
        stats.m_expertise += item_stats.m_expertise;
        stats.m_spirit += item_stats.m_spirit;
    }
}
