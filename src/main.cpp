#include "SDL3/SDL_events.h"
#include "SDL3/SDL_keycode.h"
#include "SDL3/SDL_scancode.h"
#include "game_logic/character.hpp"
#include "game_logic/gear.hpp"

#include "deltatime.hpp"
#include "pch.hpp"
#include "renderer.hpp"
#include "renderer/camera.hpp"
#include "renderer/shader.hpp"

namespace {

struct Material {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;
};

struct Light {
    glm::vec3 color;
    glm::vec3 pos;
    float ambient;
    float diffuse;
    float specular;
};

} // anonymous namespace

int main()
{
    try {
        Renderer::Window window("Test Window", 800, 600);
        SDL_SetWindowRelativeMouseMode(window.get_window_ptr(), true);

        Renderer::Camera camera(90.0, 0.1, 1000.0, window.get_aspect_ratio(), { 0.0, 0.0, 0.0 });
        camera.set_speed(5);

        DeltaTime clock;

        window.process_input_callback([&](SDL_Event& event) {
            if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                camera.update_aspect(window.get_aspect_ratio());
            }
            if (event.type == SDL_EVENT_MOUSE_MOTION) {
                camera.rotate(event.motion.xrel, -event.motion.yrel);
            }

            if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.key == SDLK_ESCAPE) {
                    window.set_should_close();
                }
            }
        });

        auto window_keystate = [&]() {
            const bool* keys = SDL_GetKeyboardState(nullptr);
            clock.update();
            float delta_time = clock.delta_time();
            if (keys[SDL_SCANCODE_W]) {
                camera.move(Renderer::Camera::Movement::Forward, delta_time);
            }
            if (keys[SDL_SCANCODE_S]) {
                camera.move(Renderer::Camera::Movement::Backward, delta_time);
            }
            if (keys[SDL_SCANCODE_A]) {
                camera.move(Renderer::Camera::Movement::Left, delta_time);
            }
            if (keys[SDL_SCANCODE_D]) {
                camera.move(Renderer::Camera::Movement::Right, delta_time);
            }
            if (keys[SDL_SCANCODE_SPACE]) {
                camera.move(Renderer::Camera::Movement::Up, delta_time);
            }
            if (keys[SDL_SCANCODE_LSHIFT]) {
                camera.move(Renderer::Camera::Movement::Down, delta_time);
            }
        };

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        glEnable(GL_MULTISAMPLE);

        std::array<Renderer::ShaderInfo, 2> shaders = {
            Renderer::ShaderInfo {
                .is_file = true,
                .shader = "res/model.glsl.vs",
                .type = GL_VERTEX_SHADER,
            },
            Renderer::ShaderInfo {
                .is_file = true,
                .shader = "res/model.glsl.fs",
                .type = GL_FRAGMENT_SHADER,
            },
        };

        Renderer::ShaderProgram shader(shaders.data(), shaders.size());

        // Renderer::Model model("res/backpack/backpack.obj");
        Renderer::Model model("res/Sponza/glTF/Sponza.gltf");
        // Renderer::Model model("res/cube/Cube.obj");

        Material material = {
            .ambient = glm::vec3(1.0),
            .diffuse = glm::vec3(1.0),
            .specular = glm::vec3(1.0),
            .shininess = 32.0,
        };

        Light light = {
            .color = glm::vec3(1.0, 1.0, 0.268),
            .pos = glm::vec3(8.0),
            .ambient = 0.1,
            .diffuse = 0.5,
            .specular = 0.5,
        };

        window.loop([&]() {
            window_keystate();
            camera.update();

            glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            shader.bind();
            shader.set_mat4("proj", camera.get_proj());
            shader.set_mat4("view", camera.get_view());
            shader.set_mat4("model", glm::scale(glm::mat4 { 1.0 }, glm::vec3(0.1)));
            shader.set_vec3("view_pos", camera.get_pos());

            shader.set_vec3("material.ambient", material.ambient);
            shader.set_vec3("material.diffuse", material.diffuse);
            shader.set_vec3("material.specular", material.specular);
            shader.set_float("material.shininess", material.shininess);

            shader.set_vec3("light.color", light.color);
            shader.set_vec3("light.pos", light.pos);
            shader.set_float("light.ambient", light.ambient);
            shader.set_float("light.diffuse", light.diffuse);
            shader.set_float("light.specular", light.specular);

            model.draw(shader);
        });
    } catch (std::runtime_error& error) {
        std::println("Caught error: {}", error.what());
    }

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

// void raylib_test()
// {
//     const int screenWidth = 1600;
//     const int screenHeight = 800;
//
//     InitWindow(screenWidth, screenHeight, "Dungeons");
//
//     SetTargetFPS(144);
//
//     // Main game loop
//     while (!WindowShouldClose()) // Detect window close button or ESC key
//     {
//         BeginDrawing();
//
//         ClearBackground(BLACK);
//
//         DrawRectangle(0, 0, 60, 60, GRAY);
//
//         Vector2 mouse_position = GetMousePosition();
//         if (mouse_position.x >= 0 && mouse_position.x <= 60 && mouse_position.y >= 0 && mouse_position.y <= 60) {
//             DrawText("W", 30, 30, 10, BLACK);
//         }
//
//         DrawText("Congrats! You created your first window!", 190, 200, 20, WHITE);
//
//         EndDrawing();
//     }
//
//     CloseWindow(); // Close window and OpenGL context
// }
