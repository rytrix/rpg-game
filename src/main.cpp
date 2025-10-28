#include "character.hpp"
#include "gear.hpp"

int main()
{
    fmt::print("Hello, world!\n");

    Character c1;

    Item chest(5,
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

    Item legs(3,
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

    c1.equip_item(chest, 2);
    c1.equip_item(legs, 6);
    c1.regen_tick(10);

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

    fmt::println("Stamina cost {}, Resource cost {}", cost.m_stamina, cost.m_resource);

    f64 effectiveness = test.get_effectiveness(c1.get_statsheet());

    fmt::println("Effectiveness {}", effectiveness);

    return 0;
}
