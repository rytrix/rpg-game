#pragma once

#include "gear.hpp"

class Ability {
public:
    explicit Ability(Statsheet<f64> scaling_power);

    [[nodiscard]] f64 get_effectiveness(const Statsheet<u64>& max_stats) const;
    [[nodiscard]] u64 get_cost(const Statsheet<u64>& max_stats) const;

private:
    Statsheet<f64> m_scaling_power;

    static constexpr u32 CRIT_CAP = 70;
    [[nodiscard]] static f64 crit_effectiveness(f64 effectiveness, f64 scaling_power, f64 crit_stat);
};

class Character {
public:

private:
    Statsheet<u64> m_max_stats = {};
    u64 m_cur_stamina = {};
    u64 m_cur_resource = {};

    static constexpr u32 ITEM_SLOTS = 14;
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
    std::array<Item, ITEM_SLOTS> m_items;

    void calculate_max_stats();
};
