#pragma once

#include "gear.hpp"

class Ability {
public:
    explicit Ability(Statsheet<f64> scaling_power);

    [[nodiscard]] f64 get_effectiveness(const Statsheet<u64>& max_stats) const;

    struct Cost {
        f64 m_stamina;
        f64 m_resource;
    };

    [[nodiscard]] Cost get_cost(const Statsheet<u64>& max_stats) const;

private:
    Statsheet<f64> m_scaling_power;

    static constexpr u32 CRIT_CAP = 70;
    [[nodiscard]] static f64 crit_effectiveness(f64 effectiveness, f64 scaling_power, f64 crit_stat);
};

class Character {
public:
    Character();

    [[nodiscard]] f64 get_cur_stamina() const;
    [[nodiscard]] f64 get_cur_resource() const;
    [[nodiscard]] const Statsheet<u64>& get_statsheet() const;

    Item equip_item(const Item& item, u32 slot);

    void reset_stamina_resource();
    void regen_tick(u32 ticks = 1);

    void debug_print();

private:
    Statsheet<u64> m_max_stats = {};

    static constexpr f64 STAMINA_SCALING = 10.0;
    static constexpr f64 RESOURCE_SCALING = 10.0;

    [[nodiscard]] f64 max_stamina() const;
    [[nodiscard]] f64 max_resource() const;

    f64 m_cur_stamina = {};
    f64 m_cur_resource = {};

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
