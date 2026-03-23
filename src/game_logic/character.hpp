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

// Item chest(5, Item::CHEST_SLOT,
//     Statsheet<u64> {
//         .m_stamina = 2,
//         .m_resource = 2,
//
//         .m_armor = 2,
//         .m_resist = 0,
//
//         .m_primary = 2,
//         .m_crit = 1,
//         .m_haste = 1,
//         .m_expertise = 3,
//
//         .m_spirit = 2,
//         .m_recovery = 2,
//     });
//
// Item legs(3, Item::LEG_SLOT,
//     Statsheet<u64> {
//         .m_stamina = 2,
//         .m_resource = 2,
//
//         .m_armor = 2,
//         .m_resist = 1,
//
//         .m_primary = 1,
//         .m_crit = 1,
//         .m_haste = 1,
//         .m_expertise = 1,
//
//         .m_spirit = 2,
//         .m_recovery = 2,
//     });

// std::string sql_command = R"(
//     SELECT * FROM ITEMS
//     WHERE ID IS 2
// )";
// Item legs(db, sql_command);

// c1.equip_item(chest);
// c1.equip_item(legs);
// c1.regen_tick(10);
//
// sqlite_cmd(db, Item::create_sql_table_cmd("items"));
// sqlite_cmd(db, chest.export_to_sql_cmd("items", 1, "chest"));
// sqlite_cmd(db, legs.export_to_sql_cmd("items", 2, "legs"));

// template <typename T = std::string&>
// int sqlite_cmd(sqlite3* db, T command)
// {
//     char* errmsg = nullptr;
//     int error = sqlite3_exec(db, command.c_str(), nullptr, nullptr, &errmsg);
//
//     if (error != SQLITE_OK) {
//         std::println("SQL error when creating table: {}", errmsg);
//         sqlite3_free(errmsg);
//         return 1;
//     }
//
//     return 0;
// }

// void average_stats_test()
// {
//     f32 average_crit = 0.0;
//     f32 average_haste = 0.0;
//     f32 average_expertise = 0.0;
//
//     for (int i = 0; i < 2000; i++) {
//         Item test = Item::random_item(5, Item::BOOT_SLOT, "item");
//
//         auto statsheet = test.get_leveled_statsheet();
//
//         average_crit += static_cast<f32>(statsheet.m_crit);
//         average_haste += static_cast<f32>(statsheet.m_haste);
//         average_expertise += static_cast<f32>(statsheet.m_expertise);
//     }
//
//     std::println("Average_crit: {}", average_crit);
//     std::println("Average_haste: {}", average_haste);
//     std::println("Average_expertise: {}", average_expertise);
// }

// void character_sqlite_ability_tests()
// {
//     sqlite3* db = nullptr;
//     int error = sqlite3_open("Gear.db", &db);
//     if (error != 0) {
//         std::println("Can't open database: {}", sqlite3_errmsg(db));
//     }
//
//     Character c1 = Character::random_character("c1", 531);
//     Character c2 = Character::random_character("c2", 500);
//
//     std::println("C1");
//     c1.debug_print();
//     std::println("\nC2");
//     c2.debug_print();
//     std::println("");
//
//     Ability test(
//         Statsheet<f64> {
//             .m_stamina = 0,
//             .m_resource = 0.02,
//
//             .m_armor = 1.00,
//             .m_resist = 1.00,
//
//             .m_primary = 1,
//             .m_crit = 1.0,
//             .m_haste = 1.0,
//             .m_expertise = 1.0,
//
//             .m_spirit = 0,
//             .m_recovery = 0,
//         },
//         Ability::PHYSICAL_DAMAGE);
//
//     Ability::Cost cost = test.get_cost(c1);
//
//     std::println("Ability:");
//     std::println("Stamina cost {}, Resource cost {}", cost.m_stamina, cost.m_resource);
//
//     f64 effectiveness = test.get_effectiveness(c1, c2);
//     std::println("Effectiveness {}", effectiveness);
//
//     effectiveness = test.get_effectiveness(c1, c2);
//     std::println("Effectiveness {}", effectiveness);
//
//     effectiveness = test.get_effectiveness(c1, c2);
//     std::println("Effectiveness {}", effectiveness);
//     std::println("");
//
//     sqlite_cmd(db, c1.create_sql_table_cmd());
//     sqlite_cmd(db, c1.export_to_sql_cmd("items", 0));
//
//     Character c3 = Character::import_from_sql_cmd(db, 0);
//     c3.debug_print();
//
//     sqlite3_close(db);
//     db = nullptr;
// }
