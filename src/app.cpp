#include "app.hpp"

App::App()
{
    m_physics_engine = new PhysicsEngine();
    LOG_TRACE("Initialized physics engine")

    m_window.init("Test Window", 1000, 800);
    m_window.set_relative_mode(true);
    LOG_TRACE("Created window \"Test Window\"");

    m_camera.init(90.0F, 0.1F, 1000.0F, m_window.get_aspect_ratio(), { -2.0F, 1.5F, 4.0F });
    m_camera.set_speed(5.0F);

    m_gpass.init(m_window.get_width(), m_window.get_height());
    m_lpass.init();

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
    m_gpass_shader.init(shader_info.data(), shader_info.size());

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
    m_lpass_shader.init(shader_info.data(), shader_info.size());

    auto shadowmap_info = Renderer::ShadowMap::get_shader_info();
    m_shadowmap_shader.init(shadowmap_info.data(), shadowmap_info.size());

    auto shadowmap_cubemap_info = Renderer::ShadowMap::get_shader_info_cubemap();
    m_shadowmap_cubemap_shader.init(shadowmap_cubemap_info.data(), shadowmap_cubemap_info.size());

    // m_model("res/models/backpack/backpack.obj");
    // m_model("res/models/Sponza/glTF/Sponza.gltf");
    // u_model = glm::scale(glm::mat4 { 1.0 }, glm::vec3(0.1));
    // m_model.init("res/models/cube_texture_mapping/Cube.obj");
    // u_model = glm::scale(glm::mat4 { 1.0 }, glm::vec3(1.0));

    m_plane.init("res/models/physics_plane/plane.obj");
    u_plane = glm::scale(glm::mat4 { 1.0 }, glm::vec3(1.0));
    {
        const auto* meshes = m_plane.get_meshes();
        JPH::BodyID plane_id = m_physics_engine->create_mesh_body(meshes);
        m_physics_engine->bodies.push_back(plane_id);
    }

    m_cube.init("res/models/physics_cube/cube.obj");
    u_cube = glm::scale(glm::mat4 { 1.0 }, glm::vec3(1.0));

    m_directional_light.init(
        true,
        glm::vec3(-0.2F, -1.0F, 0.3F),
        glm::vec3(0.1),
        glm::vec3(0.5),
        glm::vec3(0.5));

    m_point_light.init(true,
        glm::vec3(2.0F, 2.0F, 2.0F),
        glm::vec3(0.05F),
        glm::vec3(0.5F),
        glm::vec3(0.5F),
        1.0F,
        0.022F,
        0.0019F);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    keyboard_callback();

    // glEnable(GL_MULTISAMPLE);
}

App::~App()
{
    delete m_physics_engine;
    m_physics_engine = nullptr;
}

void App::keyboard_callback()
{
    m_window.process_input_callback([&](SDL_Event& event) {
        if (event.type == SDL_EVENT_WINDOW_RESIZED) {
            m_camera.update_aspect(m_window.get_aspect_ratio());
            m_gpass.reinit(m_window.get_width(), m_window.get_height());
        }
        if (event.type == SDL_EVENT_MOUSE_MOTION) {
            m_camera.rotate(event.motion.xrel, -event.motion.yrel);
        }
        if (event.type == SDL_EVENT_KEY_DOWN) {
            if (event.key.key == SDLK_ESCAPE) {
                m_window.set_should_close();
            }
            if (event.key.key == SDLK_P) {
                glm::vec3 pos = m_camera.get_pos();
                std::println("Camera_position: {}, {}, {}", pos.x, pos.y, pos.z);
            }
        }
    });
}

void App::keyboard_input()
{
    const bool* keys = SDL_GetKeyboardState(nullptr);
    float delta_time = m_clock.delta_time();
    using Dir = Renderer::Camera::Movement;
    if (keys[SDL_SCANCODE_W]) {
        m_camera.move(Dir::Forward, delta_time);
    }
    if (keys[SDL_SCANCODE_S]) {
        m_camera.move(Dir::Backward, delta_time);
    }
    if (keys[SDL_SCANCODE_A]) {
        m_camera.move(Dir::Left, delta_time);
    }
    if (keys[SDL_SCANCODE_D]) {
        m_camera.move(Dir::Right, delta_time);
    }
    if (keys[SDL_SCANCODE_SPACE]) {
        m_camera.move(Dir::Up, delta_time);
    }
    if (keys[SDL_SCANCODE_LSHIFT]) {
        m_camera.move(Dir::Down, delta_time);
    }
}

void App::frame_counter()
{
    static float time_passed;
    static u32 frames;
    static bool initialized;

    if (!initialized) {
        time_passed = 0.0F;
        frames = 0;
        initialized = true;
    } else {
        time_passed += m_clock.delta_time();
        frames += 1;
        if (time_passed >= 1.0F) {
            time_passed = 0;
            std::println("Frames: {}", frames);
            frames = 0;
        }
    }
}

void App::run()
{
    m_window.loop([&]() {
        m_clock.update();
        frame_counter();
        keyboard_input();
        m_camera.update();

        m_physics_engine->update(m_clock.delta_time());
        JPH::RVec3 box_pos = m_physics_engine->body_interface->GetCenterOfMassPosition(m_physics_engine->bodies[1]);
        u_cube[3] = glm::vec4(glm::vec3(box_pos.GetX(), box_pos.GetY(), box_pos.GetZ()), u_cube[3][3]);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        // Shadowmap pass
        glCullFace(GL_FRONT);
        // m_directional_light.shadowmap_draw(m_shadowmap_shader, u_model, [&]() {
        //     m_model.draw();
        // });
        // m_point_light.shadowmap_draw(m_shadowmap_cubemap_shader, u_model, [&]() {
        //     m_model.draw();
        // });

        m_directional_light.shadowmap_draw(m_shadowmap_shader, u_plane, [&]() {
            m_plane.draw();
        });
        m_directional_light.shadowmap_draw(m_shadowmap_shader, u_cube, [&]() {
            m_cube.draw();
        });

        m_point_light.shadowmap_draw(m_shadowmap_cubemap_shader, u_plane, [&]() {
            m_plane.draw();
        });
        m_point_light.shadowmap_draw(m_shadowmap_cubemap_shader, u_cube, [&]() {
            m_cube.draw();
        });

        // Geometry pass
        glCullFace(GL_BACK);
        m_gpass.bind();
        glViewport(0, 0, m_window.get_width(), m_window.get_height());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_gpass_shader.bind();
        m_gpass_shader.set_mat4("proj", m_camera.get_proj());
        m_gpass_shader.set_mat4("view", m_camera.get_view());

        // m_gpass_shader.set_mat4("model", u_model);
        // m_model.draw(m_gpass_shader);

        m_gpass_shader.set_mat4("model", u_plane);
        m_plane.draw(m_gpass_shader);
        m_gpass_shader.set_mat4("model", u_cube);
        m_cube.draw(m_gpass_shader);

        m_gpass.blit_depth_buffer();
        m_gpass.unbind();

        // Lighting pass
        glClear(GL_COLOR_BUFFER_BIT);
        m_lpass_shader.bind();

        m_gpass.set_uniforms(m_lpass_shader);
        m_lpass_shader.set_vec3("view_position", m_camera.get_pos());

        m_directional_light.set_uniforms(m_lpass_shader, "u_directional_light");
        m_point_light.set_uniforms(m_lpass_shader, "u_point_light");
        m_lpass.draw();
    });
}