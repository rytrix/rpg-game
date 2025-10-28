#include "character.hpp"

Ability::Ability(Statsheet<f64> scaling_power)
    : m_scaling_power(scaling_power)
{
}

[[nodiscard]] f64 Ability::get_effectiveness(const Statsheet<u64>& max_stats) const
{
    f64 effectiveness = m_scaling_power.m_primary * static_cast<f64>(max_stats.m_primary);

    effectiveness += m_scaling_power.m_stamina * static_cast<f64>(max_stats.m_stamina);

    effectiveness += m_scaling_power.m_haste * static_cast<f64>(max_stats.m_haste);

    effectiveness += m_scaling_power.m_expertise * static_cast<f64>(max_stats.m_expertise);

    effectiveness += m_scaling_power.m_spirit * static_cast<f64>(max_stats.m_spirit);

    effectiveness += m_scaling_power.m_recovery * static_cast<f64>(max_stats.m_recovery);

    effectiveness = crit_effectiveness(effectiveness, m_scaling_power.m_crit, static_cast<f64>(max_stats.m_crit));

    return effectiveness;
}

[[nodiscard]] Ability::Cost Ability::get_cost(const Statsheet<u64>& max_stats) const
{
    return {
        .m_stamina = static_cast<f64>(max_stats.m_stamina) * m_scaling_power.m_stamina,
        .m_resource = static_cast<f64>(max_stats.m_resource) * m_scaling_power.m_resource
    };
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

Character::Character()
    : m_items({})
{
}

f64 Character::get_cur_stamina() const
{
    return m_cur_stamina;
}

f64 Character::get_cur_resource() const
{
    return m_cur_resource;
}

[[nodiscard]] const Statsheet<u64>& Character::get_statsheet() const
{
    return m_max_stats;
}


Item Character::equip_item(const Item& item, const u32 slot)
{
    if (slot >= ITEM_SLOTS) {
        throw std::runtime_error(fmt::format("Invalid item slot \"{}\", should within 0-{}", slot, ITEM_SLOTS));
    }

    Item old_item = m_items.at(slot);

    m_items.at(slot) = item;

    // Update stored stat value
    // I kinda hate this (maybe put it somewhere else)
    calculate_max_stats();

    return old_item;
}

void Character::reset_stamina_resource()
{
    m_cur_stamina = max_stamina();
    m_cur_resource = max_resource();
}

void Character::regen_tick(u32 ticks)
{
    m_cur_stamina += static_cast<f64>(ticks * m_max_stats.m_recovery);
    m_cur_stamina = std::clamp(m_cur_stamina, 0.0, max_stamina());

    m_cur_resource += static_cast<f64>(ticks * m_max_stats.m_spirit);
    m_cur_resource = std::clamp(m_cur_resource, 0.0, max_resource());
}

void Character::debug_print()
{
    fmt::println("stamina: {} {}/{}", m_max_stats.m_stamina, m_cur_stamina, max_stamina());
    fmt::println("resource: {} {}/{}", m_max_stats.m_resource, m_cur_resource, max_resource());

    fmt::println("armor: {}", m_max_stats.m_armor);
    fmt::println("resist: {}", m_max_stats.m_resist);

    fmt::println("primary: {}", m_max_stats.m_primary);
    fmt::println("crit: {}", m_max_stats.m_crit);
    fmt::println("haste: {}", m_max_stats.m_haste);
    fmt::println("expertise: {}", m_max_stats.m_expertise);
    fmt::println("recovery: {}", m_max_stats.m_recovery);
    fmt::println("spirit: {}", m_max_stats.m_spirit);
}

[[nodiscard]] f64 Character::max_stamina() const
{
    return static_cast<f64>(m_max_stats.m_stamina) * STAMINA_SCALING;
}

[[nodiscard]] f64 Character::max_resource() const
{
    return static_cast<f64>(m_max_stats.m_resource) * RESOURCE_SCALING;
}

void Character::calculate_max_stats()
{
    Statsheet<u64> stats = {};
    stats.m_stamina = 1;
    stats.m_resource = 1;

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

        stats.m_recovery += item_stats.m_recovery;
        stats.m_spirit += item_stats.m_spirit;
    }

    m_max_stats = stats;
}
