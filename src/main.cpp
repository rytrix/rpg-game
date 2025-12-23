#include "SDL3/SDL_events.h"
#include "SDL3/SDL_keycode.h"
#include "SDL3/SDL_scancode.h"
#include "game_logic/character.hpp"
#include "game_logic/gear.hpp"

#include "deltatime.hpp"
#include "pch.hpp"
#include "renderer.hpp"
#include "renderer/gbuffer.hpp"
#include "renderer/quad.hpp"
#include "renderer/shader.hpp"
#include "renderer/shadowmap.hpp"

namespace {

struct Material {
    float shininess;
};

namespace Light {

    class Directional {
    public:
        Directional() = default;
        ~Directional();

        Directional(const Directional&) = delete;
        Directional& operator=(const Directional&) = delete;
        Directional(Directional&&) = default;
        Directional& operator=(Directional&&) = default;

        void init(bool shadowmap, glm::vec3 direction, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular);

        void shadowmap_draw(Renderer::ShaderProgram& shader, glm::mat4& model, const std::function<void()>& draw_function);

        void set_uniforms(Renderer::ShaderProgram& shader, const char* light_name);

        glm::vec3 m_direction {};
        glm::vec3 m_ambient {};
        glm::vec3 m_diffuse {};
        glm::vec3 m_specular {};

    private:
        bool m_shadowmap_enabled {};
        struct ShadowMap_Internal {
            glm::mat4 m_light_space_matrix {};
            Renderer::ShadowMap m_shadowmap;
        };
        ShadowMap_Internal* m_shadowmap_internal = nullptr;
    };

    Directional::~Directional()
    {
        if (m_shadowmap_enabled) {
            delete m_shadowmap_internal;
        }
    }

    void Directional::init(bool shadowmap, glm::vec3 direction, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular)
    {
        m_direction = direction;
        m_ambient = ambient;
        m_diffuse = diffuse;
        m_specular = specular;

        if (shadowmap) {
            m_shadowmap_enabled = true;

            glm::mat4 light_projection = glm::ortho(-10.0F, 10.0F, -10.F, 10.0F, 1.0F, 20.0F);
            glm::mat4 light_view = glm::lookAt(
                glm::vec3(4.0F, 6.0F, -4.0F),
                m_direction,
                glm::vec3(0.0F, 1.0F, 0.0F));

            m_shadowmap_internal = new ShadowMap_Internal();
            m_shadowmap_internal->m_light_space_matrix = light_projection * light_view;
            m_shadowmap_internal->m_shadowmap.init();
        }
    }

    void Directional::shadowmap_draw(Renderer::ShaderProgram& shader, glm::mat4& model, const std::function<void()>& draw_function)
    {
        if (m_shadowmap_enabled) {
            glViewport(0, 0, m_shadowmap_internal->m_shadowmap.get_width(), m_shadowmap_internal->m_shadowmap.get_height());
            m_shadowmap_internal->m_shadowmap.bind();

            glClear(GL_DEPTH_BUFFER_BIT);
            shader.bind();
            shader.set_mat4("light_space_matrix", m_shadowmap_internal->m_light_space_matrix);
            shader.set_mat4("model", model);
            draw_function();

            m_shadowmap_internal->m_shadowmap.unbind();
        } else {
            throw std::runtime_error("Trying to call shadowmap_draw on a directional light without a shadowmap enabled");
        }
    }

    void Directional::set_uniforms(Renderer::ShaderProgram& shader, const char* light_name)
    {
        shader.set_vec3(std::format("{}.direction", light_name).c_str(), m_direction);
        shader.set_vec3(std::format("{}.ambient", light_name).c_str(), m_ambient);
        shader.set_vec3(std::format("{}.diffuse", light_name).c_str(), m_diffuse);
        shader.set_vec3(std::format("{}.specular", light_name).c_str(), m_specular);
        if (m_shadowmap_enabled) {
            shader.set_mat4(std::format("{}.light_space_matrix", light_name).c_str(), m_shadowmap_internal->m_light_space_matrix);
            m_shadowmap_internal->m_shadowmap.get_texture().bind(4);
            shader.set_int(std::format("{}.shadow_map", light_name).c_str(), 4);
        }
    }

    class Point {
    public:
        Point() = default;
        ~Point();

        Point(const Point&) = delete;
        Point& operator=(const Point&) = delete;
        Point(Point&&) = default;
        Point& operator=(Point&&) = default;

        void init(bool shadowmap, glm::vec3 position, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float constant, float linear, float quadratic);

        void shadowmap_draw(Renderer::ShaderProgram& shader, glm::mat4& model, const std::function<void()>& draw_function);

        void set_uniforms(Renderer::ShaderProgram& shader, const char* light_name);

        glm::vec3 m_pos {};
        glm::vec3 m_ambient {};
        glm::vec3 m_diffuse {};
        glm::vec3 m_specular {};

        // https://wiki.ogre3d.org/tiki-index.php?page=-Point+Light+Attenuation
        float m_constant {};
        float m_linear {};
        float m_quadratic {};

    private:
        bool m_shadowmap_enabled {};
        struct ShadowMap_Internal {
            std::array<glm::mat4, 6> m_light_space_matrix {};
            Renderer::ShadowMap m_shadowmap;
            float m_far;
        };
        ShadowMap_Internal* m_shadowmap_internal = nullptr;
    };

    Point::~Point()
    {
        if (m_shadowmap_enabled) {
            delete m_shadowmap_internal;
        }
    }

    void Point::init(bool shadowmap,
        glm::vec3 position, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular,
        float constant, float linear, float quadratic)
    {

        m_pos = position;
        m_ambient = ambient;
        m_diffuse = diffuse;
        m_specular = specular;

        m_constant = constant;
        m_linear = linear;
        m_quadratic = quadratic;

        if (shadowmap) {
            m_shadowmap_enabled = true;

            m_shadowmap_internal = new ShadowMap_Internal();
            m_shadowmap_internal->m_shadowmap.init_cubemap();

            float aspect = m_shadowmap_internal->m_shadowmap.get_width() / m_shadowmap_internal->m_shadowmap.get_height();
            float near = 1.0F;
            float far = 25.0F;

            glm::mat4 light_projection = glm::perspective(glm::radians(90.0F), aspect, near, far);
            m_shadowmap_internal->m_light_space_matrix.at(0) = light_projection * glm::lookAt(m_pos, m_pos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
            m_shadowmap_internal->m_light_space_matrix.at(1) = light_projection * glm::lookAt(m_pos, m_pos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
            m_shadowmap_internal->m_light_space_matrix.at(2) = light_projection * glm::lookAt(m_pos, m_pos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0));
            m_shadowmap_internal->m_light_space_matrix.at(3) = light_projection * glm::lookAt(m_pos, m_pos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0));
            m_shadowmap_internal->m_light_space_matrix.at(4) = light_projection * glm::lookAt(m_pos, m_pos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0));
            m_shadowmap_internal->m_light_space_matrix.at(5) = light_projection * glm::lookAt(m_pos, m_pos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0));
            m_shadowmap_internal->m_far = far;
        }
    }

    void Point::shadowmap_draw(Renderer::ShaderProgram& shader, glm::mat4& model, const std::function<void()>& draw_function)
    {
        if (m_shadowmap_enabled) {
            glViewport(0, 0, m_shadowmap_internal->m_shadowmap.get_width(), m_shadowmap_internal->m_shadowmap.get_height());
            m_shadowmap_internal->m_shadowmap.bind();

            glClear(GL_DEPTH_BUFFER_BIT);
            shader.bind();
            shader.set_mat4("light_space_matrices[0]", m_shadowmap_internal->m_light_space_matrix[0]);
            shader.set_mat4("light_space_matrices[1]", m_shadowmap_internal->m_light_space_matrix[1]);
            shader.set_mat4("light_space_matrices[2]", m_shadowmap_internal->m_light_space_matrix[2]);
            shader.set_mat4("light_space_matrices[3]", m_shadowmap_internal->m_light_space_matrix[3]);
            shader.set_mat4("light_space_matrices[4]", m_shadowmap_internal->m_light_space_matrix[4]);
            shader.set_mat4("light_space_matrices[5]", m_shadowmap_internal->m_light_space_matrix[5]);
            shader.set_mat4("model", model);
            shader.set_float("far_plane", m_shadowmap_internal->m_far);
            shader.set_vec3("light_pos", m_pos);
            draw_function();

            m_shadowmap_internal->m_shadowmap.unbind();
        } else {
            throw std::runtime_error("Trying to call shadowmap_draw on a point light without a shadowmap enabled");
        }
    }

    void Point::set_uniforms(Renderer::ShaderProgram& shader, const char* light_name)
    {
        shader.set_vec3(std::format("{}.pos", light_name).c_str(), m_pos);
        shader.set_vec3(std::format("{}.ambient", light_name).c_str(), m_ambient);
        shader.set_vec3(std::format("{}.diffuse", light_name).c_str(), m_diffuse);
        shader.set_vec3(std::format("{}.specular", light_name).c_str(), m_specular);
        shader.set_float(std::format("{}.constant", light_name).c_str(), m_constant);
        shader.set_float(std::format("{}.linear", light_name).c_str(), m_linear);
        shader.set_float(std::format("{}.quadratic", light_name).c_str(), m_quadratic);
        if (m_shadowmap_enabled) {
            shader.set_float(std::format("{}.far_plane", light_name).c_str(), m_shadowmap_internal->m_far);
            m_shadowmap_internal->m_shadowmap.get_texture().bind(5);
            shader.set_int(std::format("{}.shadow_map", light_name).c_str(), 5);
        }
    }

} // namespace Light

} // anonymous namespace

int main()
{
    try {
        Renderer::Window window("Test Window", 800, 600);
        window.set_relative_mode(true);

        Renderer::Camera camera(90.0F, 0.1F, 1000.0F, window.get_aspect_ratio(), { -2.0F, 1.5F, 4.0F });
        camera.set_speed(5.0F);

        DeltaTime clock;

        Renderer::GBuffer gpass;
        gpass.init(window.get_width(), window.get_height());

        window.process_input_callback([&](SDL_Event& event) {
            if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                camera.update_aspect(window.get_aspect_ratio());
                gpass.reinit(window.get_width(), window.get_height());
            }
            if (event.type == SDL_EVENT_MOUSE_MOTION) {
                camera.rotate(event.motion.xrel, -event.motion.yrel);
            }
            if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.key == SDLK_ESCAPE) {
                    window.set_should_close();
                }
                if (event.key.key == SDLK_P) {
                    glm::vec3 pos = camera.get_pos();
                    std::println("Camera_position: {}, {}, {}", pos.x, pos.y, pos.z);
                }
            }
        });

        auto window_keystate = [&]() {
            const bool* keys = SDL_GetKeyboardState(nullptr);
            float delta_time = clock.delta_time();
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

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        // glEnable(GL_MULTISAMPLE);

        auto shadowmap_info = Renderer::ShadowMap::get_shader_info();
        Renderer::ShaderProgram shadowmap_shader(shadowmap_info.data(), shadowmap_info.size());

        auto shadowmap_cubemap_info = Renderer::ShadowMap::get_shader_info_cubemap();
        Renderer::ShaderProgram shadowmap_cubemap_shader(shadowmap_cubemap_info.data(), shadowmap_cubemap_info.size());

        std::array<Renderer::ShaderInfo, 2> shader_info = {
            Renderer::ShaderInfo {
                .is_file = true,
                .shader = "res/deferred_shading/g_pass.glsl.vert",
                .type = GL_VERTEX_SHADER,
            },
            Renderer::ShaderInfo {
                .is_file = true,
                .shader = "res/deferred_shading/g_pass.glsl.frag",
                .type = GL_FRAGMENT_SHADER,
            },
        };
        Renderer::ShaderProgram gpass_shader(shader_info.data(), shader_info.size());

        Renderer::Quad lpass;
        lpass.init();

        shader_info = {
            Renderer::ShaderInfo {
                .is_file = true,
                .shader = "res/deferred_shading/l_pass.glsl.vert",
                .type = GL_VERTEX_SHADER,
            },
            Renderer::ShaderInfo {
                .is_file = true,
                .shader = "res/deferred_shading/l_pass.glsl.frag",
                .type = GL_FRAGMENT_SHADER,
            },
        };
        Renderer::ShaderProgram lpass_shader(shader_info.data(), shader_info.size());

        // Renderer::Model model("res/backpack/backpack.obj");
        Renderer::Model model("res/Sponza/glTF/Sponza.gltf");
        glm::mat4 u_model = glm::scale(glm::mat4 { 1.0 }, glm::vec3(0.1));
        // Renderer::Model model("res/cube_texture_mapping/Cube.obj");
        // glm::mat4 u_model = glm::scale(glm::mat4 { 1.0 }, glm::vec3(1.0));

        Light::Directional directional_light;
        directional_light.init(
            true,
            glm::vec3(-0.2F, -1.0F, 0.3F),
            glm::vec3(0.1),
            glm::vec3(0.5),
            glm::vec3(0.5));

        Light::Point point_light;
        point_light.init(true,
            glm::vec3(2.0F, 2.0F, 2.0F),
            glm::vec3(0.05F),
            glm::vec3(0.5F),
            glm::vec3(0.5F),
            1.0F,
            0.022F,
            0.0019F);

        float time_passed = 0.0F;
        u32 frames = 0;
        auto fps_printer = [&]() {
            time_passed += clock.delta_time();
            frames += 1;
            if (time_passed >= 1.0F) {
                time_passed = 0;
                std::println("Frames: {}", frames);
                frames = 0;
            }
        };

        window.loop([&]() {
            clock.update();
            fps_printer();
            window_keystate();
            camera.update();

            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

            // Shadowmap pass
            glCullFace(GL_FRONT);
            directional_light.shadowmap_draw(shadowmap_shader, u_model, [&]() {
                model.draw();
            });

            point_light.shadowmap_draw(shadowmap_cubemap_shader, u_model, [&]() {
                model.draw();
            });

            // Geometry pass
            glCullFace(GL_BACK);
            gpass.bind();
            glViewport(0, 0, window.get_width(), window.get_height());
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            gpass_shader.bind();
            gpass_shader.set_mat4("proj", camera.get_proj());
            gpass_shader.set_mat4("view", camera.get_view());
            gpass_shader.set_mat4("model", u_model);

            model.draw(gpass_shader);

            gpass.blit_depth_buffer();

            gpass.unbind();

            // Lighting pass
            glClear(GL_COLOR_BUFFER_BIT);
            lpass_shader.bind();

            gpass.set_uniforms(lpass_shader);
            lpass_shader.set_vec3("view_position", camera.get_pos());

            directional_light.set_uniforms(lpass_shader, "u_directional_light");
            point_light.set_uniforms(lpass_shader, "u_point_light");
            lpass.draw();
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
