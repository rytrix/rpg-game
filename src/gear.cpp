#include "gear.hpp"

Item::Item(u32 item_level, u32 slot, Statsheet<u64> base_stats)
    : m_item_level(item_level)
    , m_slot(slot)
    , m_base_stats(base_stats)
{
    // if (base_stats.m_resource != 0) {
    //     throw std::runtime_error("Item resource stat is greater than zero");
    // }
}

[[nodiscard]] u32 Item::get_slot() const
{
    return m_slot;
}

[[nodiscard]] const Statsheet<u64>& Item::get_base_statsheet() const
{
    return m_base_stats;
}

[[nodiscard]] Statsheet<u64> Item::get_leveled_statsheet() const
{
    Statsheet<u64> stats = {};

    stats.m_stamina = m_base_stats.m_stamina * (ILVL_SCALING_FACTOR * m_item_level);
    stats.m_resource = m_base_stats.m_resource * (ILVL_SCALING_FACTOR * m_item_level);

    stats.m_armor = m_base_stats.m_armor * (ILVL_SCALING_FACTOR * m_item_level);
    stats.m_resist = m_base_stats.m_resist * (ILVL_SCALING_FACTOR * m_item_level);

    stats.m_primary = m_base_stats.m_primary * (ILVL_SCALING_FACTOR * m_item_level);
    stats.m_crit = m_base_stats.m_crit * (ILVL_SCALING_FACTOR * m_item_level);
    stats.m_haste = m_base_stats.m_haste * (ILVL_SCALING_FACTOR * m_item_level);
    stats.m_expertise = m_base_stats.m_expertise * (ILVL_SCALING_FACTOR * m_item_level);

    stats.m_spirit = m_base_stats.m_spirit * (ILVL_SCALING_FACTOR * m_item_level);
    stats.m_recovery = m_base_stats.m_recovery * (ILVL_SCALING_FACTOR * m_item_level);

    return stats;
}

[[nodiscard]] std::string Item::create_sql_table_cmd(const char* table_name)
{
    std::string command = std::format(
        R"(CREATE TABLE IF NOT EXISTS {} (
        id INTEGER PRIMARY KEY,
        name TEXT,
        ilvl INTEGER,
        slot INTEGER,
        stamina INTEGER,
        resource INTEGER,
        armor INTEGER,
        resist INTEGER,
        primary_stat INTEGER,
        crit INTEGER,
        haste INTEGER,
        expertise INTEGER,
        spirit INTEGER,
        recovery INTEGER);
    )",
        table_name);

    return command;
}

[[nodiscard]] std::string Item::export_to_sql_cmd(const char* table_name, int id, const char* item_name) const
{
    std::string command = std::format(
        R"(REPLACE INTO {} (
        id, name, ilvl, slot, stamina, resource, armor, resist,
        primary_stat, crit, haste, expertise, spirit, recovery
        ) VALUES (
        {}, '{}', {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {});
    )",
        table_name, id, item_name, m_item_level, m_slot, m_base_stats.m_stamina, m_base_stats.m_resource,
        m_base_stats.m_armor, m_base_stats.m_resist, m_base_stats.m_primary,
        m_base_stats.m_crit, m_base_stats.m_haste, m_base_stats.m_expertise,
        m_base_stats.m_spirit, m_base_stats.m_recovery);

    return command;
}

int Item::import_from_sql_cmd(sqlite3* database, std::string& sql_command)
{
    sqlite3_stmt* statement = nullptr;
    if (sqlite3_prepare_v2(database, sql_command.c_str(), static_cast<int>(sql_command.length()), &statement, nullptr) != SQLITE_OK) {
        std::println("Failed to prepare sqlite3 statement \"{}\"", sql_command);
        return -1;
    }

    if (sqlite3_step(statement) == SQLITE_ROW) {
        m_item_level = static_cast<u32>(sqlite3_column_int(statement, 2));
        m_slot = static_cast<u32>(sqlite3_column_int(statement, 3));
        m_base_stats.m_stamina = static_cast<u64>(sqlite3_column_int(statement, 4));
        m_base_stats.m_resource = static_cast<u64>(sqlite3_column_int(statement, 5));

        m_base_stats.m_armor = static_cast<u64>(sqlite3_column_int(statement, 6));
        m_base_stats.m_resist = static_cast<u64>(sqlite3_column_int(statement, 7));

        m_base_stats.m_primary = static_cast<u64>(sqlite3_column_int(statement, 8));
        m_base_stats.m_crit = static_cast<u64>(sqlite3_column_int(statement, 9));
        m_base_stats.m_haste = static_cast<u64>(sqlite3_column_int(statement, 10));
        m_base_stats.m_expertise = static_cast<u64>(sqlite3_column_int(statement, 11));

        m_base_stats.m_spirit = static_cast<u64>(sqlite3_column_int(statement, 12));
        m_base_stats.m_recovery = static_cast<u64>(sqlite3_column_int(statement, 13));
    }

    sqlite3_finalize(statement);

    return 0;
}

void Item::debug_print()
{
    std::println("item level: {}", m_item_level);
    std::println("slot: {}", m_slot);

    std::println("stamina: {}", m_base_stats.m_stamina);
    std::println("resource: {}", m_base_stats.m_resource);

    std::println("armor: {}", m_base_stats.m_armor);
    std::println("resist: {}", m_base_stats.m_resist);

    std::println("primary: {}", m_base_stats.m_primary);
    std::println("crit: {}", m_base_stats.m_crit);
    std::println("haste: {}", m_base_stats.m_haste);
    std::println("expertise: {}", m_base_stats.m_expertise);
    std::println("recovery: {}", m_base_stats.m_recovery);
    std::println("spirit: {}", m_base_stats.m_spirit);
}
