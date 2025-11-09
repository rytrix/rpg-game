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
    static constexpr u32 HEALING = 2;

private:
    Statsheet<f64> m_scaling_power;
    u32 m_damage_type;

    static constexpr u32 CRIT_CAP = 70;
    [[nodiscard]] static f64 crit_effectiveness(f64 effectiveness, f64 scaling_power, f64 crit_stat);
};

class Character {
public:
    explicit Character(const char* name);

    static Character random_character(const char* name, u32 item_level);

    [[nodiscard]] f64 get_cur_stamina() const;
    [[nodiscard]] f64 get_cur_resource() const;
    [[nodiscard]] const Statsheet<u64>& get_statsheet() const;
    [[nodiscard]] Statsheet<f64> get_scaled_statsheet() const;
    [[nodiscard]] f64 get_item_level() const;

    Item equip_item(const Item& item);

    void reset_stamina_resource();
    void regen_tick(u32 ticks = 1);

    [[nodiscard]] static std::string create_sql_table_cmd();
    [[nodiscard]] std::string export_to_sql_cmd(const char* item_table_name, int id) const;
    [[nodiscard]] static Character import_from_sql_cmd(sqlite3* database, int id);

    void debug_print();

private:
    std::string m_name;
    Statsheet<u64> m_max_stats = {};

    static constexpr Statsheet<f64> STAT_SCALING = {
        .m_stamina = 10.0,
        .m_resource = 1.0,

        .m_armor = 1.0,
        .m_resist = 1.0,

        .m_primary = 1,
        .m_crit = 0.01,
        .m_haste = 0.01,
        .m_expertise = 0.01,

        .m_spirit = 0.01,
        .m_recovery = 0.01,
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
    std::array<Item, Item::TOTAL_SLOTS> m_equiped;

    bool stats_need_updated = false;
    void update_max_stats();
};
