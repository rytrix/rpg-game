#include "app.hpp"
#include "Jolt/Math/Real.h"
#include "physics/helpers.hpp"

App::App()
{
    m_physics_engine = std::make_unique<PhysicsEngine>();
    LOG_TRACE("Initialized physics engine")

    m_window.init(m_title, 1000, 800);
    m_window.set_relative_mode(true);
    LOG_TRACE(std::format("Created window \"{}\"", m_title));

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

    // m_plane.init("res/models/Sponza/glTF/Sponza.gltf");
    m_plane.init("res/models/physics_plane/plane.obj");
    {
        JPH::TriangleList triangles;
        const auto* meshes = m_plane.get_meshes();
        PhysicsEngine::create_mesh_triangle_list(triangles, meshes);
        JPH::BodyID plane_id = m_physics_engine->m_body_interface->CreateAndAddBody(
            JPH::BodyCreationSettings(
                new JPH::MeshShapeSettings(triangles),
                JPH::RVec3::sZero(), JPH::Quat::sIdentity(),
                JPH::EMotionType::Static,
                Layers::NON_MOVING),
            JPH::EActivation::DontActivate);
        m_physics_engine->m_bodies.push_back(plane_id);
    }

    m_cube.init("res/models/physics_cube/cube.obj");
    {
        JPH::BodyCreationSettings cube_settings(
            new JPH::BoxShape(JPH::Vec3(0.5, 0.5, 0.5)),
            JPH::RVec3(-7.05, 20.0, -5.5),
            JPH::Quat::sIdentity(),
            JPH::EMotionType::Dynamic,
            Layers::MOVING);
        b_cube = m_physics_engine->m_body_interface->CreateAndAddBody(
            cube_settings,
            JPH::EActivation::Activate);
        m_physics_engine->m_bodies.push_back(b_cube);
    }

    Renderer::Light::DirectionalInfo directional_info;
    directional_info.direction = glm::vec3(-0.2F, -1.0F, 0.3F);
    directional_info.ambient = glm::vec3(0.1);
    directional_info.diffuse = glm::vec3(0.5);
    directional_info.specular = glm::vec3(0.5);
    directional_info.shadowmap = true;
    m_directional_light.init(directional_info);

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
    point_info.far = 50.0F;
    m_point_light.init(point_info);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    keyboard_callback();

    // glEnable(GL_MULTISAMPLE);

    m_physics_engine->optimize();

    m_clock.update();
    LOG_INFO(std::format("Finished loading in {} seconds", m_clock.delta_time()));
}

// App::~App()
// {
// }

void App::keyboard_callback()
{
    m_window.process_input_callback([&](SDL_Event& event) {
        if (event.type == SDL_EVENT_WINDOW_RESIZED) {
            m_camera.update_aspect(m_window.get_aspect_ratio());
            m_gpass.reinit(m_window.get_width(), m_window.get_height());
        }
        if (event.type == SDL_EVENT_MOUSE_MOTION) {
            if (m_capture_mouse) {
                m_camera.rotate(event.motion.xrel, -event.motion.yrel);
            }
        }
        if (event.type == SDL_EVENT_KEY_DOWN) {
            // if (event.key.key == SDLK_ESCAPE) {
            //     m_window.set_should_close();
            // }
            if (event.key.key == SDLK_ESCAPE) {
                if (m_capture_mouse) {
                    m_capture_mouse = false;
                    m_window.set_relative_mode(false);

                } else {
                    m_capture_mouse = true;
                    m_window.set_relative_mode(true);
                }
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
    if (m_capture_mouse) {
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
            auto title = std::format("{} {} fps", m_title, frames);
            m_window.set_window_title(title.c_str());
            frames = 0;
        }
    }
}

void App::imgui_run()
{
    // ImGui::ShowDemoWindow(); // Show demo window! :)

    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 20, main_viewport->WorkPos.y + 20), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiCond_FirstUseEver);

    // Main body of the Demo window starts here.
    if (!ImGui::Begin("Debug Window", nullptr, 0)) {
        // Early out if the window is collapsed, as an optimization.
        ImGui::End();
        return;
    }

    ImGui::Checkbox("Toggle physics", &m_physics_on);
    if (ImGui::CollapsingHeader("Cube 1")) {
        glm::mat4& u_model = m_cube.get_model_matrix();
        glm::vec3 cube_pos = u_model[3];
        ImGui::DragFloat3("XYZ", &cube_pos.x, 1.0F, -8.0f, 8.0f);
        m_physics_engine->m_body_interface->SetPosition(
            b_cube,
            vec3_to_vec3(cube_pos),
            JPH::EActivation::Activate);
        u_model[3] = glm::vec4(cube_pos, u_model[3][3]);
    }

    ImGui::End();
}

void App::run()
{
    m_window.loop([&]() {
        m_clock.update();
        frame_counter();
        keyboard_input();
        m_camera.update();
        imgui_run();

        if (m_physics_on) {
            m_physics_engine->update(m_clock.delta_time());
            m_cube.get_model_matrix() = mat4_to_mat4(m_physics_engine->m_body_interface->GetCenterOfMassTransform(b_cube));
        }

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        // Shadowmap pass
        glDisable(GL_CULL_FACE);

        m_directional_light.shadowmap_draw(m_shadowmap_shader, [&]() {
            m_shadowmap_shader.set_mat4("model", m_plane.get_model_matrix());
            m_plane.draw();
            m_shadowmap_shader.set_mat4("model", m_cube.get_model_matrix());
            m_cube.draw();
        });

        // glCullFace(GL_FRONT);

        m_point_light.shadowmap_draw(m_shadowmap_cubemap_shader, [&]() {
            m_shadowmap_cubemap_shader.set_mat4("model", m_plane.get_model_matrix());
            m_plane.draw();
            m_shadowmap_cubemap_shader.set_mat4("model", m_cube.get_model_matrix());
            m_cube.draw();
        });

        glEnable(GL_CULL_FACE);

        // Geometry pass
        // glCullFace(GL_BACK);
        m_gpass.bind();
        glViewport(0, 0, m_window.get_width(), m_window.get_height());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_gpass_shader.bind();
        m_gpass_shader.set_mat4("proj", m_camera.get_proj());
        m_gpass_shader.set_mat4("view", m_camera.get_view());

        m_gpass_shader.set_mat4("model", m_plane.get_model_matrix());
        m_plane.draw(m_gpass_shader);
        m_gpass_shader.set_mat4("model", m_cube.get_model_matrix());
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

        Renderer::Texture::reset_texture_units();
    });
}