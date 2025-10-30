#pragma once

#include "gear.hpp"

class Character;

class Ability {
public:
    Ability(Statsheet<f64> scaling_power, u32 damage_type);

    [[nodiscard]] f64 get_effectiveness(const Character& caster, const Character& target) const;

    struct Cost {
        f64 m_stamina;
        f64 m_resource;
    };

    [[nodiscard]] Cost get_cost(const Character& character) const;

    static constexpr u32 PHYSICAL_DAMAGE = 0;
    static constexpr u32 MAGIC_DAMAGE = 1;

private:
    Statsheet<f64> m_scaling_power;
    u32 m_damage_type;

    static constexpr u32 CRIT_CAP = 70;
    [[nodiscard]] static f64 crit_effectiveness(f64 effectiveness, f64 scaling_power, f64 crit_stat);
};

class Character {
public:
    Character();

    static Character random_character(u32 item_level);

    [[nodiscard]] f64 get_cur_stamina() const;
    [[nodiscard]] f64 get_cur_resource() const;
    [[nodiscard]] const Statsheet<u64>& get_statsheet() const;
    [[nodiscard]] Statsheet<f64> get_scaled_statsheet() const;

    [[nodiscard]] f64 get_armor_dr() const;
    [[nodiscard]] f64 get_resist_dr() const;

    Item equip_item(const Item& item);

    void reset_stamina_resource();
    void regen_tick(u32 ticks = 1);

    void debug_print();

private:
    Statsheet<u64> m_max_stats = {};

    static constexpr Statsheet<f64> STAT_SCALING = {
        .m_stamina = 10.0,
        .m_resource = 10.0,

        .m_armor = 0.02,
        .m_resist = 0.02,

        .m_primary = 1,
        .m_crit = 0.01,
        .m_haste = 0.01,
        .m_expertise = 0.01,

        .m_spirit = 0.02,
        .m_recovery = 0.02,
    };

    [[nodiscard]] f64 max_stamina() const;
    [[nodiscard]] f64 max_resource() const;

    f64 m_cur_stamina = {};
    f64 m_cur_resource = {};

    // 0  - helmet
    // 1  - shoulders
    // 2  - chest
    // 3  - wrist
    // 4  - hands
    // 5  - waist
    // 6  - legs
    // 7  - boots
    // 8  - ring
    // 9  - ring
    // 10 - trinket
    // 11 - trinket
    // 12 - weapon
    // 13 - offhand
    std::array<Item, Item::TOTAL_SLOTS> m_items;

    void calculate_max_stats();
};
