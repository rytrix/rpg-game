#include "SDL3/SDL_events.h"
#include "SDL3/SDL_keycode.h"
#include "SDL3/SDL_scancode.h"
#include "game_logic/character.hpp"
#include "game_logic/gear.hpp"

#include "deltatime.hpp"
#include "pch.hpp"
#include "renderer.hpp"
#include "renderer/shader.hpp"
#include "renderer/vertex.hpp"

namespace {

struct Material {
    float shininess;
};

namespace Light {

    const char* const SHADOWMAP_VERTEX_SHADER = R"(
    #version 460 core
    layout (location = 0) in vec3 aPos;

    uniform mat4 light_space_matrix;
    uniform mat4 model;

    void main()
    {
        gl_Position = light_space_matrix * model * vec4(aPos, 1.0);
    }
    )";

    const char* const SHADOWMAP_FRAG_SHADER = R"(
    #version 460 core
    void main()
    {
    }
    )";

    std::array<Renderer::ShaderInfo, 2> get_shadowpass_shader_info()
    {
        return std::array<Renderer::ShaderInfo, 2> {
            Renderer::ShaderInfo {
                .is_file = false,
                .shader = SHADOWMAP_VERTEX_SHADER,
                .type = GL_VERTEX_SHADER,
            },
            Renderer::ShaderInfo {
                .is_file = false,
                .shader = SHADOWMAP_FRAG_SHADER,
                .type = GL_FRAGMENT_SHADER,
            },
        };
    }

    class ShadowMap {
    public:
        ShadowMap() = default;
        void init();

        void bind();
        void unbind();

        static constexpr i32 SHADOW_WIDTH = 1024;
        static constexpr i32 SHADOW_HEIGHT = 1024;

    private:
        Renderer::Texture m_texture;
        Renderer::Framebuffer m_framebuffer;
    };

    void ShadowMap::init()
    {
        Renderer::TextureInfo shadowmap_info;
        shadowmap_info.size = Renderer::TextureSize { .width = SHADOW_WIDTH, .height = SHADOW_HEIGHT, .depth = 0 };
        shadowmap_info.internal_format = GL_DEPTH_COMPONENT24;
        m_texture.init(shadowmap_info);

        m_framebuffer.bind_texture(GL_DEPTH_ATTACHMENT, m_texture.get_id(), 0);
        m_framebuffer.bind();
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        m_framebuffer.unbind();
    }

    void ShadowMap::bind()
    {
        m_framebuffer.bind();
    }

    void ShadowMap::unbind()
    {
        m_framebuffer.unbind();
    }

    class Directional {
    public:
        void init(glm::vec3 direction, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular);

        void shadowmap_draw(Renderer::ShaderProgram shader, glm::mat4& model, std::function<void()>& draw_function);

        void set_uniforms(Renderer::ShaderProgram shader, const char* light_name) const;

        glm::vec3 m_direction;
        glm::vec3 m_ambient;
        glm::vec3 m_diffuse;
        glm::vec3 m_specular;

    private:
        glm::mat4 m_light_space_matrix;
        ShadowMap m_shadowmap;
    };

    void Directional::init(glm::vec3 direction, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular)
    {
        m_direction = direction;
        m_ambient = ambient;
        m_diffuse = diffuse;
        m_specular = specular;

        glm::mat4 light_projection = glm::ortho(-10.0F, 10.0F, -10.F, 10.0F, 1.0F, 7.5F);
        glm::mat4 light_view = glm::lookAt(
            glm::vec3(-2.0f, 4.0f, -1.0f),
            glm::vec3(0.0f, 0.0f, 0.0f), // TODO supposed to be direction??
            glm::vec3(0.0f, 1.0f, 0.0f));

        m_light_space_matrix = light_projection * light_view;

        m_shadowmap.init();
    }

    void Directional::shadowmap_draw(Renderer::ShaderProgram shader, glm::mat4& model, std::function<void()>& draw_function)
    {
        glViewport(0, 0, ShadowMap::SHADOW_WIDTH, ShadowMap::SHADOW_HEIGHT);
        m_shadowmap.bind();

        glClear(GL_DEPTH_BUFFER_BIT);
        shader.set_mat4("light_space_matrix", m_light_space_matrix);
        shader.set_mat4("model", model);
        draw_function();

        m_shadowmap.unbind();
    }

    void Directional::set_uniforms(Renderer::ShaderProgram shader, const char* light_name) const
    {
        shader.set_vec3(std::format("{}.direction", light_name).c_str(), m_direction);
        shader.set_vec3(std::format("{}.ambient", light_name).c_str(), m_ambient);
        shader.set_vec3(std::format("{}.diffuse", light_name).c_str(), m_diffuse);
        shader.set_vec3(std::format("{}.specular", light_name).c_str(), m_specular);
    }

    struct Point {
        glm::vec3 pos;
        glm::vec3 ambient;
        glm::vec3 diffuse;
        glm::vec3 specular;

        // https://wiki.ogre3d.org/tiki-index.php?page=-Point+Light+Attenuation
        float constant;
        float linear;
        float quadratic;
    };

} // namespace Light

constexpr i32 SCREEN_WIDTH = 1920;
constexpr i32 SCREEN_HEIGHT = 1080;

class GBuffer {
public:
    GBuffer() = default;
    ~GBuffer() = default;

    GBuffer(const GBuffer&) = delete;
    GBuffer& operator=(const GBuffer&) = delete;
    GBuffer(GBuffer&&) = default;
    GBuffer& operator=(GBuffer&&) = default;

    void init();
    void bind();
    void unbind();

    void set_uniforms(Renderer::ShaderProgram& shader);

private:
    Renderer::Framebuffer m_buffer;

    Renderer::Texture m_position;
    Renderer::Texture m_normal;
    Renderer::Texture m_albedo;

    Renderer::Renderbuffer m_depth;
};

void GBuffer::init()
{
    Renderer::TextureInfo texture_info;
    texture_info.size = Renderer::TextureSize { .width = SCREEN_WIDTH, .height = SCREEN_HEIGHT, .depth = 0 };
    texture_info.internal_format = GL_RGBA16F;
    texture_info.mipmaps = GL_FALSE;
    m_position.init(texture_info);
    m_buffer.bind_texture(GL_COLOR_ATTACHMENT0, m_position.get_id(), 0);

    m_normal.init(texture_info);
    m_buffer.bind_texture(GL_COLOR_ATTACHMENT1, m_normal.get_id(), 0);

    texture_info.internal_format = GL_RGBA16F;
    m_albedo.init(texture_info);
    m_buffer.bind_texture(GL_COLOR_ATTACHMENT2, m_albedo.get_id(), 0);

    m_depth.buffer_storage(GL_DEPTH_COMPONENT24, SCREEN_WIDTH, SCREEN_HEIGHT);

    m_buffer.bind_renderbuffer(GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depth.get_id());

    std::array<u32, 3> attachments = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    m_buffer.bind_draw_buffers(attachments.size(), attachments.data());
}

void GBuffer::bind()
{
    m_buffer.bind();
}

void GBuffer::set_uniforms(Renderer::ShaderProgram& shader)
{
    m_position.bind(0);
    shader.set_int("gPosition", 0);

    m_normal.bind(1);
    shader.set_int("gNormal", 1);

    m_albedo.bind(2);
    shader.set_int("gAlbedoSpec", 2);
}

void GBuffer::unbind()
{
    m_buffer.unbind();
}

class Quad {
public:
    Quad();

    void init();
    void draw();

private:
    Renderer::VertexArray m_vao;
    Renderer::Buffer m_vbo;
};

Quad::Quad()
{
    init();
}

void Quad::init()
{
    m_vao.vertex_attrib(0, 0, 3, GL_FLOAT, 0);
    m_vao.vertex_attrib(1, 0, 2, GL_FLOAT, 3 * sizeof(float));
    // clang-format off
    std::array<float, 20> quad_vertices = {
        // positions        // texture Coords
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };
    // clang-format on 
    m_vbo.buffer_data(quad_vertices.size() * sizeof(float), quad_vertices.data(), GL_STATIC_DRAW);
    m_vao.bind_vertex_buffer(0, m_vbo.get_id(), 0, 5 * sizeof(float));
}

void Quad::draw()
{
    m_vao.bind();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

} // anonymous namespace

int main()
{
    try {
        Renderer::Window window("Test Window", 800, 600);
        window.set_relative_mode(true);

        Renderer::Camera camera(90.0F, 0.1F, 1000.0F,
            window.get_aspect_ratio(), { -2.0F, 1.5F, 4.0F });
        camera.set_speed(5.0F);

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
                if (event.key.key == SDLK_P) {
                    glm::vec3 pos = camera.get_pos();
                    std::println("Camera_position: {}, {}, {}", pos.x, pos.y, pos.z);
                }
            }
        });

        auto window_keystate = [&]() {
            const bool* keys = SDL_GetKeyboardState(nullptr);
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

        // glEnable(GL_MULTISAMPLE);

        std::array<Renderer::ShaderInfo, 2> shader_info = {
            Renderer::ShaderInfo {
                .is_file = true,
                .shader = "res/forward_pass/model.glsl.vs",
                .type = GL_VERTEX_SHADER,
            },
            Renderer::ShaderInfo {
                .is_file = true,
                .shader = "res/forward_pass/model_light_map.glsl.fs",
                .type = GL_FRAGMENT_SHADER,
            },
        };

        Renderer::ShaderProgram model_shader(shader_info.data(), shader_info.size());

        // auto shadowmap_info = Light::get_shadowpass_shader_info();
        // Renderer::ShaderProgram shadowmap_shader(shadowmap_info.data(), shadowmap_info.size());

        GBuffer gpass;
        gpass.init();

        shader_info = {
            Renderer::ShaderInfo {
                .is_file = true,
                .shader = "res/deferred_shading/g_pass.glsl.vs",
                .type = GL_VERTEX_SHADER,
            },
            Renderer::ShaderInfo {
                .is_file = true,
                .shader = "res/deferred_shading/g_pass.glsl.fs",
                .type = GL_FRAGMENT_SHADER,
            },
        };
        Renderer::ShaderProgram gpass_shader(shader_info.data(), shader_info.size());

        Quad lpass;
        lpass.init();
        shader_info = {
            Renderer::ShaderInfo {
                .is_file = true,
                .shader = "res/deferred_shading/l_pass.glsl.vs",
                .type = GL_VERTEX_SHADER,
            },
            Renderer::ShaderInfo {
                .is_file = true,
                .shader = "res/deferred_shading/l_pass.glsl.fs",
                .type = GL_FRAGMENT_SHADER,
            },
        };
        Renderer::ShaderProgram lpass_shader(shader_info.data(), shader_info.size());

        // Renderer::Model model("res/backpack/backpack.obj");
        Renderer::Model model("res/Sponza/glTF/Sponza.gltf");
        glm::mat4 u_model = glm::scale(glm::mat4 { 1.0 }, glm::vec3(0.1));
        // Renderer::Model model("res/cube_texture_mapping/Cube.obj");
        // glm::mat4 u_model = glm::scale(glm::mat4 { 1.0 }, glm::vec3(1.0));

        Material material = {
            .shininess = 32.0F,
        };

        Light::Point point_light = {
            .pos = glm::vec3(12.0F, 11.0F, 14.6F),
            .ambient = glm::vec3(0.1F),
            .diffuse = glm::vec3(0.5F),
            .specular = glm::vec3(0.5F),
            .constant = 1.0F,
            .linear = 0.022F,
            .quadratic = 0.0019F,
        };

        // DirectionalLight directional_light = {
        //     .direction = glm::vec3(-0.2F, -1.0F, 0.3F),
        //     .ambient = glm::vec3(0.01),
        //     .diffuse = glm::vec3(0.5),
        //     .specular = glm::vec3(0.5),
        // };

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

        bool forward_pass = false;
        window.loop([&]() {
            clock.update();
            fps_printer();
            window_keystate();
            camera.update();

            if (forward_pass) {

                glViewport(0, 0, window.get_size().first, window.get_size().second);
                glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                model_shader.bind();
                model_shader.set_mat4("proj", camera.get_proj());
                model_shader.set_mat4("view", camera.get_view());
                model_shader.set_mat4("model", u_model);
                model_shader.set_vec3("view_pos", camera.get_pos());

                model_shader.set_float("material.shininess", material.shininess);

                // shader.set_vec3("u_directional_light.direction", directional_light.direction);
                // shader.set_vec3("u_directional_light.ambient", directional_light.ambient);
                // shader.set_vec3("u_directional_light.diffuse", directional_light.diffuse);
                // shader.set_vec3("u_directional_light.specular", directional_light.specular);

                model_shader.set_vec3("u_point_light.pos", point_light.pos);
                model_shader.set_vec3("u_point_light.ambient", point_light.ambient);
                model_shader.set_vec3("u_point_light.diffuse", point_light.diffuse);
                model_shader.set_vec3("u_point_light.specular", point_light.specular);
                model_shader.set_float("u_point_light.constant", point_light.constant);
                model_shader.set_float("u_point_light.linear", point_light.linear);
                model_shader.set_float("u_point_light.quadratic", point_light.quadratic);

                model.draw(model_shader);
            } else {
                glViewport(0, 0, window.get_size().first, window.get_size().second);
                glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

                // Geometry pass
                gpass.bind();
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                gpass_shader.bind();
                gpass_shader.set_mat4("proj", camera.get_proj());
                gpass_shader.set_mat4("view", camera.get_view());
                gpass_shader.set_mat4("model", u_model);

                model.draw(gpass_shader);

                gpass.unbind();

                // Lighting pass
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                lpass_shader.bind();

                gpass.set_uniforms(lpass_shader);
                lpass_shader.set_vec3("view_position", camera.get_pos());

                lpass_shader.set_vec3("u_directional_light.direction", glm::vec3(-0.2F, -1.0F, 0.3F));
                lpass_shader.set_vec3("u_directional_light.ambient", glm::vec3(0.1));
                lpass_shader.set_vec3("u_directional_light.diffuse", glm::vec3(0.5));
                lpass_shader.set_vec3("u_directional_light.specular", glm::vec3(0.5));
                lpass.draw();
            }
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
