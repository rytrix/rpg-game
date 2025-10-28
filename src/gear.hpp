#pragma once

template <typename T>
struct Statsheet {
    T m_stamina = {};
    T m_resource = {};

    T m_armor = {};
    T m_resist = {};

    T m_primary = {};
    T m_crit = {};
    T m_haste = {};
    T m_expertise = {};

    T m_spirit = {};
    T m_recovery = {};
};

class Item {
public:
    Item() = default;
    Item(u32 item_level, u32 slot, Statsheet<u64> base_stats);

    [[nodiscard]] u32 get_slot() const;
    [[nodiscard]] const Statsheet<u64>& get_base_statsheet() const;
    [[nodiscard]] Statsheet<u64> get_leveled_statsheet() const;

    [[nodiscard]] static std::string create_sql_table_cmd(const char* table_name);
    [[nodiscard]] std::string export_to_sql_cmd(const char* table_name, int id, const char* item_name) const;
    int import_from_sql_cmd(sqlite3* database, std::string& sql_command);

    static constexpr u32 HELMET_SLOT = 0;
    static constexpr u32 SHOULDER_SLOT = 1;
    static constexpr u32 CHEST_SLOT = 2;
    static constexpr u32 WRIST_SLOT = 3;
    static constexpr u32 HAND_SLOT = 4;
    static constexpr u32 WAIST_SLOT = 5;
    static constexpr u32 LEG_SLOT = 6;
    static constexpr u32 BOOT_SLOT = 7;
    static constexpr u32 RING_SLOT1 = 8;
    static constexpr u32 RING_SLOT2 = 9;
    static constexpr u32 TRINKET_SLOT1 = 10;
    static constexpr u32 TRINKET_SLOT2 = 11;
    static constexpr u32 WEAPON_SLOT = 12;
    static constexpr u32 OFFHAND_SLOT = 13;

    void debug_print();

private:
    u32 m_item_level = {};
    u32 m_slot = {};
    Statsheet<u64> m_base_stats = {};

    static constexpr u64 ILVL_SCALING_FACTOR = 1.0;
};
