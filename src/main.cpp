#include "game_logic/character.hpp"
#include "game_logic/gear.hpp"

#include "app.hpp"

int main()
{
    try {
        App app;
        app.run();
    } catch (std::runtime_error& error) {
        std::println("Caught error: {}", error.what());
    }

    return 0;
}

// Renderer::Window window("Test Window", 800, 600);
// window.set_relative_mode(true);

// Renderer::Camera camera(90.0F, 0.1F, 1000.0F, window.get_aspect_ratio(), { -2.0F, 1.5F, 4.0F });
// camera.set_speed(5.0F);

// Utils::DeltaTime clock;

// Renderer::GBuffer gpass;
// gpass.init(window.get_width(), window.get_height());

// auto shadowmap_info = Renderer::ShadowMap::get_shader_info();
// Renderer::ShaderProgram shadowmap_shader(shadowmap_info.data(), shadowmap_info.size());

// auto shadowmap_cubemap_info = Renderer::ShadowMap::get_shader_info_cubemap();
// Renderer::ShaderProgram shadowmap_cubemap_shader(shadowmap_cubemap_info.data(), shadowmap_cubemap_info.size());

// std::array<Renderer::ShaderInfo, 2> shader_info = {
//     Renderer::ShaderInfo {
//         .is_file = true,
//         .shader = "res/deferred_shading/g_pass.glsl.vert",
//         .type = GL_VERTEX_SHADER,
//     },
//     Renderer::ShaderInfo {
//         .is_file = true,
//         .shader = "res/deferred_shading/g_pass.glsl.frag",
//         .type = GL_FRAGMENT_SHADER,
//     },
// };
// Renderer::ShaderProgram gpass_shader(shader_info.data(), shader_info.size());

// Renderer::Quad lpass;
// lpass.init();

// shader_info = {
//     Renderer::ShaderInfo {
//         .is_file = true,
//         .shader = "res/deferred_shading/l_pass.glsl.vert",
//         .type = GL_VERTEX_SHADER,
//     },
//     Renderer::ShaderInfo {
//         .is_file = true,
//         .shader = "res/deferred_shading/l_pass.glsl.frag",
//         .type = GL_FRAGMENT_SHADER,
//     },
// };
// Renderer::ShaderProgram lpass_shader(shader_info.data(), shader_info.size());

// // Renderer::Model model("res/backpack/backpack.obj");
// // Renderer::Model model("res/Sponza/glTF/Sponza.gltf");
// // glm::mat4 u_model = glm::scale(glm::mat4 { 1.0 }, glm::vec3(0.1));
// Renderer::Model model("res/cube_texture_mapping/Cube.obj");
// glm::mat4 u_model = glm::scale(glm::mat4 { 1.0 }, glm::vec3(1.0));

// Renderer::Light::Directional directional_light;
// directional_light.init(
//     true,
//     glm::vec3(-0.2F, -1.0F, 0.3F),
//     glm::vec3(0.1),
//     glm::vec3(0.5),
//     glm::vec3(0.5));

// Renderer::Light::Point point_light;
// point_light.init(true,
//     glm::vec3(2.0F, 2.0F, 2.0F),
//     glm::vec3(0.05F),
//     glm::vec3(0.5F),
//     glm::vec3(0.5F),
//     1.0F,
//     0.022F,
//     0.0019F);

// auto window_keystate = [&]() {
//     const bool* keys = SDL_GetKeyboardState(nullptr);
//     float delta_time = clock.delta_time();
//     using Dir = Renderer::Camera::Movement;
//     if (keys[SDL_SCANCODE_W]) {
//         camera.move(Dir::Forward, delta_time);
//     }
//     if (keys[SDL_SCANCODE_S]) {
//         camera.move(Dir::Backward, delta_time);
//     }
//     if (keys[SDL_SCANCODE_A]) {
//         camera.move(Dir::Left, delta_time);
//     }
//     if (keys[SDL_SCANCODE_D]) {
//         camera.move(Dir::Right, delta_time);
//     }
//     if (keys[SDL_SCANCODE_SPACE]) {
//         camera.move(Dir::Up, delta_time);
//     }
//     if (keys[SDL_SCANCODE_LSHIFT]) {
//         camera.move(Dir::Down, delta_time);
//     }
// };

// glEnable(GL_DEPTH_TEST);
// glDepthFunc(GL_LESS);

// glEnable(GL_CULL_FACE);
// glCullFace(GL_BACK);

// // glEnable(GL_MULTISAMPLE);

// float time_passed = 0.0F;
// u32 frames = 0;
// auto fps_printer = [&]() {
//     time_passed += clock.delta_time();
//     frames += 1;
//     if (time_passed >= 1.0F) {
//         time_passed = 0;
//         std::println("Frames: {}", frames);
//         frames = 0;
//     }
// };

// window.loop([&]() {
//     clock.update();
//     fps_printer();
//     window_keystate();
//     camera.update();

//     glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

//     // Shadowmap pass
//     glCullFace(GL_FRONT);
//     directional_light.shadowmap_draw(shadowmap_shader, u_model, [&]() {
//         model.draw();
//     });

//     point_light.shadowmap_draw(shadowmap_cubemap_shader, u_model, [&]() {
//         model.draw();
//     });

//     // Geometry pass
//     glCullFace(GL_BACK);
//     gpass.bind();
//     glViewport(0, 0, window.get_width(), window.get_height());
//     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//     gpass_shader.bind();
//     gpass_shader.set_mat4("proj", camera.get_proj());
//     gpass_shader.set_mat4("view", camera.get_view());
//     gpass_shader.set_mat4("model", u_model);

//     model.draw(gpass_shader);

//     gpass.blit_depth_buffer();

//     gpass.unbind();

//     // Lighting pass
//     glClear(GL_COLOR_BUFFER_BIT);
//     lpass_shader.bind();

//     gpass.set_uniforms(lpass_shader);
//     lpass_shader.set_vec3("view_position", camera.get_pos());

//     directional_light.set_uniforms(lpass_shader, "u_directional_light");
//     point_light.set_uniforms(lpass_shader, "u_point_light");
//     lpass.draw();
// });

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
