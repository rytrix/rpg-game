#include "app.hpp"

#include "renderer/text.hpp"

#include "utils/math/ray.hpp"

#include "physics/interface.hpp"

App::App()
{
    Physics::Engine::setup_singletons();

    m_app_data.m_window.init("Test Window", 800, 600);
    m_app_data.m_window.set_relative_mode(m_app_data.m_capture_mouse);

    m_app_data.m_camera.init(90.0F, 1.0F, 500.0F, m_app_data.m_window.get_aspect_ratio(), { -2.0F, 1.5F, 4.0F });
    m_app_data.m_camera.set_speed(10.0F);

    m_app_data.m_model_cache.init(100);
    m_app_data.m_texture_cache.init(500);

    m_app_data.m_default_textures.init(&m_app_data.m_texture_cache);

    m_app_data.m_text_renderer.init("res/fonts/AdwaitaSans-Regular.ttf", 24);
    m_app_data.m_text_renderer.update_view(m_app_data.m_window.get_width(), m_app_data.m_window.get_height());

    m_app_data.m_line_renderer.init();

    m_scene = new Scene(&m_app_data);

    m_app_data.m_gizmo.init(&m_app_data);
    m_app_data.m_entity_selector.init(m_scene, &m_app_data);

    m_app_data.m_window.process_input_callback([&](SDL_Event& event) {
        if (event.type == SDL_EVENT_WINDOW_RESIZED) {
            m_app_data.m_camera.update_aspect(m_app_data.m_window.get_aspect_ratio());
            m_scene->update();
            m_app_data.m_text_renderer.update_view((float)m_app_data.m_window.get_width(), (float)m_app_data.m_window.get_height());
        }
        if (event.type == SDL_EVENT_MOUSE_MOTION) {
            if (m_app_data.m_capture_mouse) {
                m_app_data.m_camera.rotate(event.motion.xrel, -event.motion.yrel);
            }
        }
        if (event.type == SDL_EVENT_KEY_DOWN) {
            if (event.key.key == SDLK_ESCAPE) {
                m_app_data.m_capture_mouse = !m_app_data.m_capture_mouse;
                m_app_data.m_window.set_relative_mode(m_app_data.m_capture_mouse);
                m_app_data.m_entity_selector.m_selected_entity = Entity(m_scene, entt::null);
            }
            if (event.key.key == SDLK_E) {
                m_scene->m_physics_on = !m_scene->m_physics_on;
            }
            if (event.key.key == SDLK_Q) {
                m_app_data.m_window.set_should_close();
            }
        }

        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            if (event.button.button == SDL_BUTTON_LEFT) {
                if (!m_app_data.m_entity_selector.m_selected_entity.valid() && m_app_data.m_entity_selector.m_hovered_entity.valid()) {
                    m_app_data.m_entity_selector.m_selected_entity = m_app_data.m_entity_selector.m_hovered_entity;
                    m_app_data.m_gizmo.m_state = Gizmo::State::Translation;
                }
            }
        }

        Event engine_event {};
        engine_event.m_type = Event::Type::SDL;
        engine_event.m_sdl_event = event;
        engine_event.m_consumed = false;

        m_app_data.m_gizmo.on_event(engine_event);
        m_app_data.m_entity_selector.on_event(engine_event);
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
    Entity::add_model(entity, "res/models/Sponza/glTF/Sponza.gltf", &m_app_data);
    // Entity::add_model(entity, "res/models/physics_plane/plane.obj", &m_data);
    Transform transform {};
    transform.set_scale(glm::vec3(0.1));
    Entity::add_transform(entity, transform);
    Entity::add_static_body(entity);

    entity = m_scene->create_entity();
    Entity::add_name(entity, "Cube");
    Entity::add_model(entity, "res/models/physics_cube/cube.obj", &m_app_data);
    transform = {};
    transform.set_position(glm::vec3(0.0, 5.0, 0.0));
    Entity::add_transform(entity, transform);
    JPH::Ref<JPH::Shape> box_shape = new JPH::BoxShape(JPH::Vec3(0.5, 0.5, 0.5));
    Entity::add_dynamic_body(entity, box_shape);

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
    Entity::add_model(entity, "res/models/Defeated.fbx", &m_app_data);
    Entity::add_transform(entity, transform);

    entity = m_scene->create_entity();
    transform = {};
    transform.set_scale(glm::vec3(10.0F));
    transform.rotate(-90.0F, glm::vec3(1.0, 0.0, 0.0));
    Entity::add_name(entity, "Dog");
    Entity::add_model(entity, "res/models/dog/scene.gltf", &m_app_data);
    Entity::add_transform(entity, transform);

    Renderer::SkyboxInfo skybox_info {};
    skybox_info.file = "res/skyboxes/Cubemap_Sky_14-512x512.png";
    m_scene->add_component<Renderer::Skybox>(skybox_info);

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
            frames = 0;
        }
    }
}

void App::run()
{
    auto scancodes = [&]() {
        if (m_app_data.m_capture_mouse) {
            const bool* keys = SDL_GetKeyboardState(nullptr);
            float delta_time = m_scene->get_clock().delta_time<float>();
            using Dir = Renderer::Camera::Movement;
            if (keys[SDL_SCANCODE_W]) {
                m_app_data.m_camera.move(Dir::Forward, delta_time);
            }
            if (keys[SDL_SCANCODE_S]) {
                m_app_data.m_camera.move(Dir::Backward, delta_time);
            }
            if (keys[SDL_SCANCODE_A]) {
                m_app_data.m_camera.move(Dir::Left, delta_time);
            }
            if (keys[SDL_SCANCODE_D]) {
                m_app_data.m_camera.move(Dir::Right, delta_time);
            }
            if (keys[SDL_SCANCODE_SPACE]) {
                m_app_data.m_camera.move(Dir::Up, delta_time);
            }
            if (keys[SDL_SCANCODE_LSHIFT]) {
                m_app_data.m_camera.move(Dir::Down, delta_time);
            }
        }
    };

    m_app_data.m_window.loop([&]() {
        fps_counter();

        scancodes();

        m_scene->update();

        m_app_data.m_entity_selector.update();

        m_scene->draw();

        m_app_data.m_entity_selector.draw();

        m_app_data.m_line_renderer.draw(m_app_data.m_camera);

        m_app_data.m_text_renderer.draw_text(10,
            m_app_data.m_window.get_height() - m_app_data.m_text_renderer.get_max_pixel_height(),
            std::format("Framerate {}", m_fps).c_str(), glm::vec3 { 1.0F });

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
                m_app_data.m_window.set_swap_interval(1);
            } else {
                m_app_data.m_window.set_swap_interval(0);
            }
        }

        ImGui::Checkbox("Toggle physics", &m_scene->m_physics_on);

        if (m_app_data.m_entity_selector.m_selected_entity.valid()) {
            if (ImGui::CollapsingHeader("Gizmo")) {
                if (ImGui::Button("Gizmo Translation")) {
                    m_app_data.m_gizmo.m_state = Gizmo::State::Translation;
                }
                ImGui::SameLine();
                if (ImGui::Button("Gizmo Rotation")) {
                    m_app_data.m_gizmo.m_state = Gizmo::State::Rotation;
                }
                if (!m_app_data.m_entity_selector.m_selected_entity.has_component<Physics::PhysicsInfo>()
                    || (m_app_data.m_entity_selector.m_selected_entity.has_component<Physics::PhysicsInfo>()
                        && m_app_data.m_entity_selector.m_selected_entity.get_component<Physics::PhysicsInfo>().m_motion_type == JPH::EMotionType::Static)) {
                    ImGui::SameLine();
                    if (ImGui::Button("Gizmo Scale")) {
                        m_app_data.m_gizmo.m_state = Gizmo::State::Scale;
                    }
                }
                if (ImGui::Button("Deselect Entity")) {
                    m_app_data.m_entity_selector.m_selected_entity = Entity(m_scene, entt::null);
                }
            }
        }

        if (ImGui::CollapsingHeader("Scene_1")) {
            m_scene->draw_debug_imgui();
        }

        ImGui::End();
    });
}
