#include "app.hpp"

#include "renderer/text.hpp"

App::App()
{
    Physics::Engine::setup_singletons();

    m_data.m_window.init(m_window_title, 800, 600);
    m_data.m_window.set_relative_mode(m_capture_mouse);

    m_data.m_camera.init(90.0F, 1.0F, 500.0F, m_data.m_window.get_aspect_ratio(), { -2.0F, 1.5F, 4.0F });
    m_data.m_camera.set_speed(10.0F);

    m_data.m_model_cache.init(100);
    m_data.m_texture_cache.init(500);

    m_data.m_default_textures.init(&m_data.m_texture_cache);

    m_data.text_renderer.init("res/fonts/AdwaitaSans-Regular.ttf", 24);
    m_data.text_renderer.update_view(m_data.m_window.get_width(), m_data.m_window.get_height());
    m_scene = new Scene(&m_data);

    m_data.m_window.process_input_callback([&](SDL_Event& event) {
        if (event.type == SDL_EVENT_WINDOW_RESIZED) {
            m_data.m_camera.update_aspect(m_data.m_window.get_aspect_ratio());
            m_scene->update();
            m_data.text_renderer.update_view(m_data.m_window.get_width(), m_data.m_window.get_height());
        }
        if (event.type == SDL_EVENT_MOUSE_MOTION) {
            if (m_capture_mouse) {
                m_data.m_camera.rotate(event.motion.xrel, -event.motion.yrel);
            }
        }
        if (event.type == SDL_EVENT_KEY_DOWN) {
            if (event.key.key == SDLK_ESCAPE) {
                m_capture_mouse = !m_capture_mouse;
                m_data.m_window.set_relative_mode(m_capture_mouse);
            }
            if (event.key.key == SDLK_E) {
                m_physics_on = !m_physics_on;
            }
            if (event.key.key == SDLK_P) {
                glm::vec3 pos = m_data.m_camera.get_pos();
                std::println("Camera_position: {}, {}, {}", pos.x, pos.y, pos.z);
            }
            if (event.key.key == SDLK_Q) {
                m_data.m_window.set_should_close();
            }
        }
    });

    auto entity = m_scene->create_entity();
    Renderer::Light::Pbr::Directional directional {};
    directional.direction = glm::vec3(-0.2F, -1.0F, 0.3F);
    directional.color = glm::vec3(0.8);
    Entity::add_name(entity, "Directional Light");
    Entity::add_pbr_directional_light(entity, directional);
    Entity::add_pbr_directional_light_shadow(entity);

    entity = m_scene->create_entity();
    Entity::add_name(entity, "Sponza Model");
    Entity::add_model(entity, "res/models/Sponza/glTF/Sponza.gltf", &m_data);
    // Entity::add_model(entity, "res/models/physics_plane/plane.obj", &m_data);
    Transform transform {};
    transform.set_scale(glm::vec3(0.1));
    Entity::add_transform(entity, transform);
    Entity::add_physics_command(entity, [&](Physics::System* system, Renderer::Model* model) -> std::pair<JPH::BodyID, JPH::EMotionType> {
        JPH::TriangleList triangles;
        const auto* mesh = model->get_mesh();
        Physics::System::create_mesh_triangle_list_base_index(triangles, transform.get_model(), mesh);
        // Physics::System::create_mesh_triangle_list_base_index(triangles, mesh);
        JPH::BodyID plane_id = system->m_body_interface->CreateAndAddBody(
            JPH::BodyCreationSettings(
                new JPH::MeshShapeSettings(triangles),
                JPH::RVec3::sZero(), JPH::Quat::sIdentity(),
                JPH::EMotionType::Static,
                Physics::Layers::NON_MOVING),
            JPH::EActivation::DontActivate);
        return { plane_id, JPH::EMotionType::Static };
    });

    entity = m_scene->create_entity();
    Entity::add_name(entity, "Cube");
    Entity::add_model(entity, "res/models/physics_cube/cube.obj", &m_data);
    transform = {};
    Entity::add_transform(entity, transform);
    Entity::add_physics_command(entity, [](Physics::System* system, [[maybe_unused]] Renderer::Model* _model) -> std::pair<JPH::BodyID, JPH::EMotionType> {
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

    // for (int i = 0; i <= 50; i++) {
    //     e3.add_physics_command([](Physics::System* system, [[maybe_unused]] Renderer::Model* _model) -> std::pair<JPH::BodyID, JPH::EMotionType> {
    //         float y = rand() % 300;
    //         float x = rand() % 10 - 5;
    //         float z = rand() % 10 - 5;
    //         JPH::BodyCreationSettings cube_settings(
    //             new JPH::BoxShape(JPH::Vec3(0.5, 0.5, 0.5)),
    //             JPH::RVec3(x, y, z),
    //             JPH::Quat::sIdentity(),
    //             JPH::EMotionType::Dynamic,
    //             Physics::Layers::MOVING);
    //         auto body = system->m_body_interface->CreateAndAddBody(
    //             cube_settings,
    //             JPH::EActivation::Activate);
    //         return { body, JPH::EMotionType::Dynamic };
    //     });
    //     m_scene->add_entity(e3);
    // }

    entity = m_scene->create_entity();
    Renderer::Light::Pbr::Point point {};
    point.position = glm::vec3(6.0F, 6.0F, 8.0F);
    point.color = glm::vec3(10.0, 10.0, 10.0);
    Entity::add_name(entity, "Point light 1");
    Entity::add_pbr_point_light(entity, point);
    Entity::add_pbr_point_light_shadow(entity);

    entity = m_scene->create_entity();
    point.position = glm::vec3(6.0F, 6.0F, -8.0F);
    point.color = glm::vec3(50.0, 25.0, 25.0);
    Entity::add_name(entity, "Point light 2");
    Entity::add_pbr_point_light(entity, point);
    Entity::add_pbr_point_light_shadow(entity);

    entity = m_scene->create_entity();
    Renderer::Light::Pbr::Spot spot {};
    spot.position = glm::vec3(-6.0F, 8.0F, 10.0F);
    spot.direction = glm::vec3(0.2, 0.0, -1.0);
    spot.color = glm::vec3(50.0, 25.0, 25.0);
    spot.inner_cutoff = glm::cos(glm::radians(12.5F));
    spot.outer_cutoff = glm::cos(glm::radians(20.5F));
    Entity::add_name(entity, "Spot Light");
    Entity::add_pbr_spot_light(entity, spot);
    Entity::add_pbr_spot_light_shadow(entity);

    entity = m_scene->create_entity();
    transform = {};
    transform.set_scale(glm::vec3(0.1));
    Entity::add_name(entity, "Defeated");
    Entity::add_model(entity, "res/models/Defeated.fbx", &m_data);
    Entity::add_transform(entity, transform);

    entity = m_scene->create_entity();
    transform = {};
    transform.set_scale(glm::vec3(0.1));
    transform.set_scale(glm::vec3(10.0F));
    transform.rotate(-90.0F, glm::vec3(1.0, 0.0, 0.0));
    Entity::add_name(entity, "Dog");
    Entity::add_model(entity, "res/models/dog/scene.gltf", &m_data);
    Entity::add_transform(entity, transform);

    m_scene->optimize();
    m_scene->update();
}

App::~App()
{
    delete m_scene;
    Physics::Engine::cleanup_singletons();
}

void App::fps_counter()
{
    static float time_passed;
    static u32 frames;
    static bool initialized;

    if (!initialized) {
        time_passed = 0.0F;
        frames = 0;
        initialized = true;
    } else {
        time_passed += m_scene->get_clock().delta_time<float>();
        frames += 1;
        if (time_passed >= 1.0F) {
            time_passed = 0;
            m_fps = frames;
            auto title = std::format("{} {} fps", m_window_title, frames);
            m_data.m_window.set_window_title(title.c_str());
            frames = 0;
        }
    }
}

void App::run()
{
    auto scancodes = [&]() {
        if (m_capture_mouse) {
            const bool* keys = SDL_GetKeyboardState(nullptr);
            float delta_time = m_scene->get_clock().delta_time<float>();
            using Dir = Renderer::Camera::Movement;
            if (keys[SDL_SCANCODE_W]) {
                m_data.m_camera.move(Dir::Forward, delta_time);
            }
            if (keys[SDL_SCANCODE_S]) {
                m_data.m_camera.move(Dir::Backward, delta_time);
            }
            if (keys[SDL_SCANCODE_A]) {
                m_data.m_camera.move(Dir::Left, delta_time);
            }
            if (keys[SDL_SCANCODE_D]) {
                m_data.m_camera.move(Dir::Right, delta_time);
            }
            if (keys[SDL_SCANCODE_SPACE]) {
                m_data.m_camera.move(Dir::Up, delta_time);
            }
            if (keys[SDL_SCANCODE_LSHIFT]) {
                m_data.m_camera.move(Dir::Down, delta_time);
            }
        }
    };

    m_data.m_window.loop([&]() {
        m_scene->update();
        scancodes();
        fps_counter();

        if (m_physics_on) {
            m_scene->physics();
        }

        m_scene->draw();

        m_data.text_renderer.draw_text(10, m_data.m_window.get_height() - m_data.text_renderer.get_max_pixel_height(), std::format("Framerate {}", m_fps).c_str(), glm::vec3 { 1.0F });

        const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 20, main_viewport->WorkPos.y + 20), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiCond_FirstUseEver);

        // Main body of the Demo window starts here.
        if (!ImGui::Begin("Debug Window", nullptr, 0)) {
            // Early out if the window is collapsed, as an optimization.
            ImGui::End();
            return;
        }

        if (ImGui::Checkbox("Toggle vsync", &m_vsync)) {
            LOG_INFO(std::format("Setting swap interval to {}", m_vsync));
            if (m_vsync) {
                m_data.m_window.set_swap_interval(1);
            } else {
                m_data.m_window.set_swap_interval(0);
            }
        }

        ImGui::Checkbox("Toggle physics", &m_physics_on);

        if (ImGui::CollapsingHeader("Scene_1")) {
            m_scene->draw_debug_imgui();
        }

        ImGui::End();
    });
}
