#include "game_logic/character.hpp"
#include "game_logic/gear.hpp"

#include "app.hpp"
#include "scene/scene.hpp"

int main()
{
    Renderer::Window window;
    window.init("Hi", 1000, 800);
    window.set_relative_mode(true);

    Physics::Engine::setup_singletons();
    Scene scene(window);

    EntityBuilder e1;
    Renderer::Light::DirectionalInfo directional_info;
    directional_info.direction = glm::vec3(-0.2F, -1.0F, 0.3F);
    directional_info.ambient = glm::vec3(0.1);
    directional_info.diffuse = glm::vec3(0.5);
    directional_info.specular = glm::vec3(0.5);
    directional_info.shadowmap = true;
    e1.add_name("directional_light_1");
    e1.add_directional_light(directional_info);
    scene.add_entity(e1);

    EntityBuilder e2;
    e2.add_name("plane");
    e2.add_model_path("res/models/physics_plane/plane.obj");
    e2.add_physics_command([](Physics::System* system, Renderer::Model* model) -> std::pair<JPH::BodyID, JPH::EMotionType> {
        JPH::TriangleList triangles;
        const auto* meshes = model->get_meshes();
        Physics::System::create_mesh_triangle_list(triangles, meshes);
        JPH::BodyID plane_id = system->m_body_interface->CreateAndAddBody(
            JPH::BodyCreationSettings(
                new JPH::MeshShapeSettings(triangles),
                JPH::RVec3::sZero(), JPH::Quat::sIdentity(),
                JPH::EMotionType::Static,
                Physics::Layers::NON_MOVING),
            JPH::EActivation::DontActivate);
        return { plane_id, JPH::EMotionType::Static };
    });
    scene.add_entity(e2);

    EntityBuilder e3;
    e3.add_name("cube");
    e3.add_model_path("res/models/physics_cube/cube.obj");
    e3.add_physics_command([](Physics::System* system, [[maybe_unused]] Renderer::Model* _model) -> std::pair<JPH::BodyID, JPH::EMotionType> {
        JPH::BodyCreationSettings cube_settings(
            new JPH::BoxShape(JPH::Vec3(0.5, 0.5, 0.5)),
            JPH::RVec3(-7.05, 20.0, -5.5),
            JPH::Quat::sIdentity(),
            JPH::EMotionType::Dynamic,
            Physics::Layers::MOVING);
        auto body = system->m_body_interface->CreateAndAddBody(
            cube_settings,
            JPH::EActivation::Activate);
        return { body, JPH::EMotionType::Dynamic };
    });
    scene.add_entity(e3);

    for (int i = 0; i <= 50; i++) {
        e3.add_physics_command([](Physics::System* system, [[maybe_unused]] Renderer::Model* _model) -> std::pair<JPH::BodyID, JPH::EMotionType> {
            float y = rand() % 100;
            JPH::BodyCreationSettings cube_settings(
                new JPH::BoxShape(JPH::Vec3(0.5, 0.5, 0.5)),
                JPH::RVec3(7.05, y, -5.5),
                JPH::Quat::sIdentity(),
                JPH::EMotionType::Dynamic,
                Physics::Layers::MOVING);
            auto body = system->m_body_interface->CreateAndAddBody(
                cube_settings,
                JPH::EActivation::Activate);
            return { body, JPH::EMotionType::Dynamic };
        });
        scene.add_entity(e3);
    }

    EntityBuilder e4;
    Renderer::Light::PointInfo point_info;
    point_info.position = glm::vec3(2.0F, 2.0F, 2.0F);
    point_info.ambient = glm::vec3(0.05F);
    point_info.diffuse = glm::vec3(0.5F);
    point_info.specular = glm::vec3(0.5F);
    point_info.constant = 1.0F;
    point_info.linear = 0.022F;
    point_info.quadratic = 0.0019F;
    point_info.shadowmap = true;
    point_info.near = 0.1F;
    point_info.far = 25.0F;
    e4.add_point_light(point_info);
    scene.add_entity(e4);

    EntityBuilder e5;
    point_info.position = glm::vec3(-2.0F, 2.0F, -2.0F);
    point_info.ambient = glm::vec3(0.00F);
    point_info.diffuse = glm::vec3(0.5F);
    point_info.specular = glm::vec3(0.5F);
    point_info.constant = 1.0F;
    point_info.linear = 0.022F;
    point_info.quadratic = 0.0019F;
    point_info.shadowmap = true;
    point_info.near = 0.1F;
    point_info.far = 25.0F;
    e5.add_point_light(point_info);
    scene.add_entity(e5);

    scene.update();
    auto& camera = scene.get_camera();

    window.process_input_callback([&](SDL_Event& event) {
        if (event.type == SDL_EVENT_WINDOW_RESIZED) {
            camera.update_aspect(window.get_aspect_ratio());
            scene.update();
        }
        if (event.type == SDL_EVENT_MOUSE_MOTION) {
            camera.rotate(event.motion.xrel, -event.motion.yrel);
        }
    });

    auto scancodes = [&]() {
        const bool* keys = SDL_GetKeyboardState(nullptr);
        float delta_time = scene.get_clock().delta_time();
        using Dir = Renderer::Camera::Movement;
        if (keys[SDL_SCANCODE_W]) {
            camera.move(Dir::Forward, delta_time);
        }
        if (keys[SDL_SCANCODE_S]) {
            camera.move(Dir::Backward, delta_time);
        }
        if (keys[SDL_SCANCODE_A]) {
            camera.move(Dir::Left, delta_time);
        }
        if (keys[SDL_SCANCODE_D]) {
            camera.move(Dir::Right, delta_time);
        }
        if (keys[SDL_SCANCODE_SPACE]) {
            camera.move(Dir::Up, delta_time);
        }
        if (keys[SDL_SCANCODE_LSHIFT]) {
            camera.move(Dir::Down, delta_time);
        }
    };

    window.loop([&]() {
        scene.update();
        scancodes();
        scene.physics();

        scene.draw();
    });

    Physics::Engine::cleanup_singletons();

    // App app;
    // app.run();

    LOG_TRACE("Exiting main function");

    return 0;
}

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
