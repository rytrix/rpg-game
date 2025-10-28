#include "character.hpp"
#include "gear.hpp"

template <typename T = std::string&>
int sqlite_cmd(sqlite3* db, T command)
{
    char* errmsg = nullptr;
    int error = sqlite3_exec(db, command.c_str(), nullptr, nullptr, &errmsg);

    if (error != SQLITE_OK) {
        std::println("SQL error when creating table: {}", errmsg);
        sqlite3_free(errmsg);
        return 1;
    }

    return 0;
}

int main()
{
    std::print("Hello, world!\n");

    sqlite3* db = nullptr;
    int error = sqlite3_open("Gear.db", &db);
    if (error != 0) {
        std::println("Can't open database: {}", sqlite3_errmsg(db));
    }

    Character c1;

    Item chest(5, Item::CHEST_SLOT,
        Statsheet<u64> {
            .m_stamina = 2,
            .m_resource = 2,

            .m_armor = 2,
            .m_resist = 0,

            .m_primary = 2,
            .m_crit = 1,
            .m_haste = 1,
            .m_expertise = 3,

            .m_spirit = 2,
            .m_recovery = 2,
        });

    Item legs(3, Item::LEG_SLOT,
        Statsheet<u64> {
            .m_stamina = 2,
            .m_resource = 2,

            .m_armor = 2,
            .m_resist = 1,

            .m_primary = 1,
            .m_crit = 1,
            .m_haste = 1,
            .m_expertise = 1,

            .m_spirit = 2,
            .m_recovery = 2,
        });

    c1.equip_item(chest);
    c1.equip_item(legs);
    c1.regen_tick(10);

    sqlite_cmd(db, Item::create_sql_table_cmd("items"));
    sqlite_cmd(db, chest.export_to_sql_cmd("items", 1, "chest"));
    sqlite_cmd(db, legs.export_to_sql_cmd("items", 2, "legs"));

    c1.debug_print();

    Ability test(Statsheet<f64> {
        .m_stamina = 0,
        .m_resource = 2,

        .m_armor = 0,
        .m_resist = 0,

        .m_primary = 1,
        .m_crit = 1,
        .m_haste = 0,
        .m_expertise = 1,

        .m_spirit = 0,
        .m_recovery = 0,
    });

    Ability::Cost cost = test.get_cost(c1.get_statsheet());

    std::println("Stamina cost {}, Resource cost {}", cost.m_stamina, cost.m_resource);

    f64 effectiveness = test.get_effectiveness(c1.get_statsheet());

    std::println("Effectiveness {}", effectiveness);

    sqlite3_close(db);
    db = nullptr;

    return 0;
}
