#include "character.hpp"

#include "gear.hpp"

Ability::Ability(Statsheet<f64> scaling_power, u32 damage_type)
    : m_scaling_power(scaling_power)
    , m_damage_type(damage_type)
{
}

[[nodiscard]] f64 Ability::get_effectiveness(const Character& caster, const Character& target) const
{
    Statsheet<f64> caster_stats = caster.get_scaled_statsheet();

    f64 effectiveness {};

    effectiveness += m_scaling_power.m_primary * caster_stats.m_primary;
    effectiveness += m_scaling_power.m_stamina * caster_stats.m_stamina;

    effectiveness *= 1.0 + (m_scaling_power.m_haste * caster_stats.m_haste / 100.0);
    effectiveness *= 1.0 + (m_scaling_power.m_expertise * caster_stats.m_expertise / 100.0);

    effectiveness *= 1.0 + (m_scaling_power.m_spirit * caster_stats.m_spirit / 100.0);
    effectiveness *= 1.0 + (m_scaling_power.m_recovery * caster_stats.m_recovery / 100.0);

    effectiveness = crit_effectiveness(effectiveness, m_scaling_power.m_crit, caster_stats.m_crit);

    Statsheet<f64> target_stats = target.get_scaled_statsheet();
    if (m_damage_type == PHYSICAL_DAMAGE) {
        f64 armor = target_stats.m_armor; //- caster_stats.m_armor_pen;
        armor = std::max(armor, 0.0);
        // std::println("Armor: {}", armor);

        f64 reduced_damage = std::clamp(armor, 0.0, effectiveness * 0.9);
        // std::println("reduced_damage clamped {}", reduced_damage);

        effectiveness -= reduced_damage;

    } else if (m_damage_type == MAGIC_DAMAGE) {
        f64 resist = target_stats.m_resist; //- caster_stats.m_resist_pen;
        resist = std::max(resist, 0.0);
        // std::println("resist: {}", resist);

        f64 reduced_damage = std::clamp(resist, 0.0, effectiveness * 0.9);
        // std::println("reduced_damage clamped {}", reduced_damage);

        effectiveness -= reduced_damage;
    }

    return effectiveness;
}

[[nodiscard]] Ability::Cost Ability::get_cost(const Character& character) const
{
    Statsheet<f64> stats = character.get_scaled_statsheet();
    return {
        .m_stamina = static_cast<f64>(stats.m_stamina) * m_scaling_power.m_stamina,
        .m_resource = static_cast<f64>(stats.m_resource) * m_scaling_power.m_resource
    };
}

[[nodiscard]] f64 Ability::crit_effectiveness(f64 effectiveness, f64 scaling_power, f64 crit_stat)
{
    f64 crit_chance = crit_stat;
    f64 overflow_crit = 0.0;

    if (crit_chance >= CRIT_CAP) {
        overflow_crit = crit_chance - CRIT_CAP;
        crit_chance = CRIT_CAP;
    }

    static thread_local std::mt19937 generator(std::random_device {}());
    std::uniform_int_distribution<int> distribution(0, 100);
    int value = distribution(generator);

    if (crit_chance <= value) {
        // std::println("{} * ({} + {} / 100.0)", effectiveness, scaling_power, overflow_crit);
        return effectiveness * (scaling_power + overflow_crit / 100.0);
    } else {
        return effectiveness;
    }
}

Character::Character()
    : m_items({})
{
}

Character Character::random_character(u32 item_level)
{
    Character random_character {};

    for (u32 i = 0; i < Item::TOTAL_SLOTS; i++) {
        Item random_item = Item::random_item(item_level, i);
        random_character.equip_item(random_item);
    }

    return random_character;
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

[[nodiscard]] f64 Character::max_stamina() const
{
    f64 max_stamina = static_cast<f64>(m_max_stats.m_stamina);
    // max_stamina += max_stamina * (static_cast<f64>(m_max_stats.m_crit) * STAT_SCALING.m_crit / 100.0);
    // max_stamina += max_stamina * (static_cast<f64>(m_max_stats.m_haste) * STAT_SCALING.m_haste / 100.0);
    // max_stamina += max_stamina * (static_cast<f64>(m_max_stats.m_expertise) * STAT_SCALING.m_expertise / 100.0);
    return max_stamina * STAT_SCALING.m_stamina;
}

[[nodiscard]] f64 Character::max_resource() const
{
    return static_cast<f64>(m_max_stats.m_resource) * STAT_SCALING.m_resource;
}

[[nodiscard]] Statsheet<f64> Character::get_scaled_statsheet() const
{
    Statsheet<f64> stats {};

    stats.m_stamina = max_stamina();
    stats.m_resource = max_resource();

    stats.m_armor = static_cast<f64>(m_max_stats.m_armor) * STAT_SCALING.m_armor;
    stats.m_resist = static_cast<f64>(m_max_stats.m_resist) * STAT_SCALING.m_resist;

    stats.m_primary = static_cast<f64>(m_max_stats.m_primary) * STAT_SCALING.m_primary;
    stats.m_crit = static_cast<f64>(m_max_stats.m_crit) * STAT_SCALING.m_crit;
    stats.m_haste = static_cast<f64>(m_max_stats.m_haste) * STAT_SCALING.m_haste;
    stats.m_expertise = static_cast<f64>(m_max_stats.m_expertise) * STAT_SCALING.m_expertise;

    stats.m_recovery = static_cast<f64>(m_max_stats.m_recovery) * STAT_SCALING.m_recovery;
    stats.m_spirit = static_cast<f64>(m_max_stats.m_spirit) * STAT_SCALING.m_spirit;

    return stats;
}

// [[nodiscard]] f64 Character::get_armor_dr() const
// {
//     // TODO get rid of this I think
//     // f64 damage_reduction = static_cast<f64>(m_max_stats.m_armor) * STAT_SCALING.m_armor;
//     f64 armor = static_cast<f64>(m_max_stats.m_armor);// - (static_cast<f64>(m_max_stats.m_stamina) * 0.5);
//
//     f64 damage_reduction = (0.9 - std::pow(std::numbers::e, -STAT_SCALING.m_armor * armor)) * 100.0;
//
//     damage_reduction = std::clamp(damage_reduction, 0.0, 90.0);
//
//     return damage_reduction;
// }
//
// [[nodiscard]] f64 Character::get_resist_dr() const
// {
//     // TODO get rid of this I think
//     // f64 damage_reduction = static_cast<f64>(m_max_stats.m_resist) * STAT_SCALING.m_resist;
//     f64 resist = static_cast<f64>(m_max_stats.m_resist);// - (static_cast<f64>(m_max_stats.m_stamina) * 0.5);
//
//     f64 damage_reduction = (0.9 - std::pow(std::numbers::e, -STAT_SCALING.m_resist * resist)) * 100.0;
//     std::println("{} = (1.0 - {} ^ {} * {}) * 100.0", damage_reduction, std::numbers::e, STAT_SCALING.m_resist, resist);
//
//     damage_reduction = std::clamp(damage_reduction, 0.0, 90.0);
//
//     return damage_reduction;
// }

Item Character::equip_item(const Item& item)
{
    if (item.get_slot() >= Item::TOTAL_SLOTS) {
        throw std::runtime_error(std::format("Invalid item slot \"{}\", should within 0-{}", item.get_slot(), Item::TOTAL_SLOTS));
    }

    Item old_item = m_items.at(item.get_slot());

    m_items.at(item.get_slot()) = item;

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
    Statsheet<f64> stats = get_scaled_statsheet();

    m_cur_stamina += ticks * max_stamina() * stats.m_recovery;
    m_cur_stamina = std::clamp(m_cur_stamina, 0.0, max_stamina());

    m_cur_resource += ticks * max_resource() * stats.m_spirit;
    m_cur_resource = std::clamp(m_cur_resource, 0.0, max_resource());
}

void Character::debug_print()
{
    Statsheet<f64> stats = get_scaled_statsheet();

    std::println("stamina: {} {}/{}", m_max_stats.m_stamina, m_cur_stamina, max_stamina());
    std::println("resource: {} {}/{}", m_max_stats.m_resource, m_cur_resource, max_resource());

    std::println("armor: {}, {}", m_max_stats.m_armor, stats.m_armor);
    std::println("resist: {}, {}", m_max_stats.m_resist, stats.m_resist);

    std::println("primary: {}, {}", m_max_stats.m_primary, stats.m_primary);
    std::println("crit: {}, {}%", m_max_stats.m_crit, stats.m_crit);
    std::println("haste: {}, {}%", m_max_stats.m_haste, stats.m_haste);
    std::println("expertise: {}, {}%", m_max_stats.m_expertise, stats.m_expertise);

    std::println("recovery: {}, {}%", m_max_stats.m_recovery, stats.m_recovery);
    std::println("spirit: {}, {}%", m_max_stats.m_spirit, stats.m_spirit);
}

void Character::calculate_max_stats()
{
    Statsheet<u64> stats = {};
    stats.m_stamina = 1;
    stats.m_resource = 1;

    for (auto& item : m_items) {
        const auto item_stats = item.get_leveled_statsheet();

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
