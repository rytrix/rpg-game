#include "scene.hpp"

#include "../physics/helpers.hpp"

#include "glm/gtc/quaternion.hpp"
#include "scene_shaders.hpp"

#include "../renderer/random_sampling_texture.hpp"

Scene::Scene(GlobalAppData* app_data)
    : m_app_data(app_data)
    , m_camera_speed(app_data->m_camera.get_speed())
    , m_random_sampling_texture(Renderer::RandomSamplingTexture::create(16, 8, 2, &app_data->m_texture_cache))
{
    m_physics_system = std::make_unique<Physics::System>();

    update();
}

Scene::~Scene()
{
    auto view = m_registry.view<JPH::BodyID>();
    for (auto [entity, body] : view.each()) {
        m_physics_system->m_body_interface->RemoveBody(body);
        m_physics_system->m_body_interface->DestroyBody(body);
    }
}

void Scene::add_entity(const EntityBuilder& entity_builder)
{
    auto entity = m_registry.create();
    if (entity_builder.m_name != nullptr) {
        m_registry.emplace<const char*>(entity, entity_builder.m_name);
    }

    if (entity_builder.m_model_path != nullptr) {
        auto* model_cache = &m_app_data->m_model_cache;
        auto handle = model_cache->get_or_create(entity_builder.m_model_path, entity_builder.m_model_path, m_app_data);
        auto* model = model_cache->get(handle);

        if (model->has_bones()) {
            m_registry.emplace<Renderer::AnimationData>(entity);
            auto& data = m_registry.get<Renderer::AnimationData>(entity);
            for (auto& animation : model->get_animations()) {
                data.data.emplace_back(animation.create_per_animation_data());
            }
        }

        m_registry.emplace<Renderer::Model*>(entity, model);
        // TODO: Decide if I want physics objects without models someday
        if (entity_builder.m_create_body != nullptr) {
            auto physics_info
                = entity_builder.m_create_body(m_physics_system.get(), m_registry.get<Renderer::Model*>(entity));
            m_registry.emplace<JPH::BodyID>(entity, physics_info.first);
            m_registry.emplace<JPH::EMotionType>(entity, physics_info.second);
            m_physics_needs_optimize = true;
        }
        m_models_instance_draw_cache_needs_update = true;
    }

    if (entity_builder.m_pbr_directional != nullptr) {
        m_registry.emplace<Renderer::Light::Pbr::Directional>(entity, *entity_builder.m_pbr_directional);
        if (entity_builder.m_pbr_directional_shadow) {
            m_registry.emplace<Renderer::Light::Pbr::DirectionalShadow>(entity);
            m_registry.get<Renderer::Light::Pbr::DirectionalShadow>(entity).init();
        }
        m_shaders_need_update = true;
    }

    if (entity_builder.m_pbr_point != nullptr) {
        m_registry.emplace<Renderer::Light::Pbr::Point>(entity, *entity_builder.m_pbr_point);
        if (entity_builder.m_pbr_point_shadow) {
            m_registry.emplace<Renderer::Light::Pbr::PointShadow>(entity);
            m_registry.get<Renderer::Light::Pbr::PointShadow>(entity).init();
        }
        m_shaders_need_update = true;
    }

    if (entity_builder.m_pbr_spot != nullptr) {
        m_registry.emplace<Renderer::Light::Pbr::Spot>(entity, *entity_builder.m_pbr_spot);
        if (entity_builder.m_pbr_spot_shadow) {
            m_registry.emplace<Renderer::Light::Pbr::SpotShadow>(entity);
            m_registry.get<Renderer::Light::Pbr::SpotShadow>(entity).init();
        }
        m_shaders_need_update = true;
    }

    m_registry.emplace<Transform>(entity, entity_builder.m_transform);
}

void Scene::optimize()
{
    m_physics_system->optimize();
    m_physics_needs_optimize = false;
}

void Scene::update()
{
    m_clock.update();

    compile_shaders();
}

void Scene::physics()
{
    m_physics_system->update(m_clock.delta_time<float>());

    auto view = m_registry.view<Transform, JPH::BodyID, JPH::EMotionType>();

    for (auto [entity, transform, body, motion] : view.each()) {
        if (motion != JPH::EMotionType::Static) {
            auto& model = transform.get_model_ref();
            model = mat4_to_mat4(m_physics_system->m_body_interface->GetCenterOfMassTransform(body));

            auto* point_light = m_registry.try_get<Renderer::Light::Pbr::Point>(entity);
            if (point_light != nullptr) {
                point_light->position = model[3];
            }

            auto* spot_light = m_registry.try_get<Renderer::Light::Pbr::Spot>(entity);
            if (spot_light != nullptr) {
                spot_light->position = model[3];
            }
        }
    }
}

void Scene::draw()
{
    if (m_models_instance_draw_cache_needs_update) {
        auto model_view = m_registry.view<Transform, Renderer::Model*>();

        m_models_instance_draw_cache.clear();
        for (auto [entity, transform, model] : model_view.each()) {

            bool contained = false;
            for (auto& model_cached : m_models_instance_draw_cache) {
                if (model == model_cached.m_model) {
                    contained = true;
                }
            }

            if (!contained) {
                m_models_instance_draw_cache.emplace_back(model, transform.get_model());
            }

            for (auto& model_cached : m_models_instance_draw_cache) {
                if (model == model_cached.m_model) {
                    model_cached.m_model_matrices.emplace_back(transform.get_model());

                    if (model_cached.m_model->has_bones()) {
                        auto& data = m_registry.get<Renderer::AnimationData>(entity);
                        auto* animation_data = data.data.at(data.selected_animation);
                        model_cached.m_animation_data.emplace_back(animation_data);
                    }
                }
            }
        }

        LOG_INFO("Updated scene instanced draw cache");
        m_models_instance_draw_cache_needs_update = false;
    } else {
        auto model_view = m_registry.view<Transform, Renderer::Model*>();

        for (auto& model : m_models_instance_draw_cache) {
            model.m_model_matrices.clear();
        }
        for (auto [entity, transform, model] : model_view.each()) {
            for (usize j = 0; j < m_models_instance_draw_cache.size(); j++) {
                if (model == m_models_instance_draw_cache[j].m_model) {
                    m_models_instance_draw_cache[j].m_model_matrices.emplace_back(transform.get_model());
                }
            }
        }
    }

    // TODO:
    // Currently selected animation needs to be no longer included in model/mesh class
    // likely will need to pass the Animation* itself through the update function
    for (auto& model : m_models_instance_draw_cache) {
        for (auto* animation_data : model.m_animation_data) {
            if (!animation_data->m_paused) {
                animation_data->m_animation_time += m_clock.delta_time<float>();
            }
            // animation_data->m_animation_time += animation_time;
        }
        model.m_model->update(model.m_model_matrices, model.m_animation_data);
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.0F, 0.0F, 0.0F, 1.0F);

    m_app_data->m_camera.update();

    glCullFace(GL_FRONT);
    auto directional_shadow_view = m_registry.view<Renderer::Light::Pbr::Directional, Renderer::Light::Pbr::DirectionalShadow>();
    for (auto [entity, light, shadow] : directional_shadow_view.each()) {
        shadow.update(light, m_app_data->m_camera);
        shadow.shadowmap_begin();
        for (auto& model : m_models_instance_draw_cache) {
            shadow.shadowmap_draw(model.m_model);
        }
        shadow.shadowmap_end();
    }

    auto point_shadow_view = m_registry.view<Renderer::Light::Pbr::Point, Renderer::Light::Pbr::PointShadow>();
    for (auto [entity, light, shadow] : point_shadow_view.each()) {
        shadow.update(light);
        shadow.shadowmap_begin();
        for (auto& model : m_models_instance_draw_cache) {
            Renderer::ShaderProgram& shader = model.m_model->has_bones() ? m_shadowmap_cubemap_shader_bones : m_shadowmap_cubemap_shader;
            shadow.shadowmap_draw(shader, light, model.m_model);
        }
        shadow.shadowmap_end();
    }

    auto spot_shadow_view = m_registry.view<Renderer::Light::Pbr::Spot, Renderer::Light::Pbr::SpotShadow>();
    for (auto [entity, light, shadow] : spot_shadow_view.each()) {
        shadow.update(light);
        shadow.shadowmap_begin();
        for (auto& model : m_models_instance_draw_cache) {
            Renderer::ShaderProgram& shader = model.m_model->has_bones() ? m_shadowmap_shader_bones : m_shadowmap_shader;
            shadow.shadowmap_draw(shader, model.m_model);
        }
        shadow.shadowmap_end();
    }

    glCullFace(GL_BACK);
    glViewport(0, 0, m_app_data->m_window.get_width(), m_app_data->m_window.get_height());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (auto& model : m_models_instance_draw_cache) {
        Renderer::ShaderProgram& shader = model.m_model->has_bones() ? m_shader_bones : m_shader;
        shader.bind();
        shader.set_mat4("proj", m_app_data->m_camera.get_proj());
        shader.set_mat4("view", m_app_data->m_camera.get_view());
        shader.set_vec3("view_position", m_app_data->m_camera.get_pos());

        auto pbr_directional_view = m_registry.view<Renderer::Light::Pbr::Directional>();
        auto pbr_point_view = m_registry.view<Renderer::Light::Pbr::Point>();
        auto pbr_spot_view = m_registry.view<Renderer::Light::Pbr::Spot>();
        u32 i = 0;
        for (auto [entity, light] : pbr_directional_view.each()) {
            light.set_uniforms(shader, std::format("u_directional_light_{}", i).c_str());
            auto* shadow = m_registry.try_get<Renderer::Light::Pbr::DirectionalShadow>(entity);
            if (shadow != nullptr) {
                shadow->set_uniforms(shader, std::format("u_directional_light_shadow_{}", i).c_str());
            }
            i++;
        }
        i = 0;
        for (auto [entity, light] : pbr_point_view.each()) {
            light.set_uniforms(shader, std::format("u_point_light_{}", i).c_str());
            auto* shadow = m_registry.try_get<Renderer::Light::Pbr::PointShadow>(entity);
            if (shadow != nullptr) {
                shadow->set_uniforms(shader, std::format("u_point_light_shadow_{}", i).c_str());
            }
            i++;
        }
        i = 0;
        for (auto [entity, light] : pbr_spot_view.each()) {
            light.set_uniforms(shader, std::format("u_spot_light_{}", i).c_str());
            auto* shadow = m_registry.try_get<Renderer::Light::Pbr::SpotShadow>(entity);
            if (shadow != nullptr) {
                shadow->set_uniforms(shader, std::format("u_spot_light_shadow_{}", i).c_str());
            }
            i++;
        }

        m_random_sampling_texture.bind_uniforms(shader, "tex_random_offset", &m_app_data->m_texture_cache);

        model.m_model->draw(shader);

        Renderer::Texture::drop_texture_units(1);
    }

    Renderer::Texture::reset_texture_units();
}

void Scene::draw_debug_imgui()
{
    if (ImGui::Button("Reload shaders")) {
        m_shaders_need_update = true;
    }

    glm::vec3 cam_pos = m_app_data->m_camera.get_pos();
    ImGui::Text("%s", std::format("Camera Pos: {}, {}, {}", cam_pos.x, cam_pos.y, cam_pos.z).c_str());
    if (ImGui::DragFloat("Camera Speed", &m_camera_speed, 0.1F, 1.0F, 20.0F)) {
        m_app_data->m_camera.set_speed(m_camera_speed);
    }

    constexpr float MAX_TRANSFORM = 64.0F;
    constexpr float MIN_TRANSFORM = -64.0F;

    constexpr float MAX_ROTATION = 360.0F;
    constexpr float MIN_ROTATION = -360.0F;

    constexpr float MAX_COLOR = 3000.0F;
    constexpr float MIN_COLOR = 0.0F;

    i32 i = 0;
    if (ImGui::CollapsingHeader("Entities")) {
        auto view = m_registry.view<entt::entity>();
        for (auto [entity] : view.each()) {
            ImGui::PushID(i);

            const char** name_check = m_registry.try_get<const char*>(entity);
            const char* entity_name = name_check == nullptr ? "no_name" : *name_check;
            auto name = std::format("{} e{}", entity_name, i);

            if (ImGui::CollapsingHeader(name.c_str())) {
                auto* try_model = m_registry.try_get<Renderer::Model*>(entity);
                auto* try_animation_data = m_registry.try_get<Renderer::AnimationData>(entity);
                auto* try_transform = m_registry.try_get<Transform>(entity);

                auto* try_body_id = m_registry.try_get<JPH::BodyID>(entity);
                auto* try_motion_type = m_registry.try_get<JPH::EMotionType>(entity);

                auto* try_point = m_registry.try_get<Renderer::Light::Pbr::Point>(entity);
                auto* try_directional = m_registry.try_get<Renderer::Light::Pbr::Directional>(entity);
                auto* try_spot = m_registry.try_get<Renderer::Light::Pbr::Spot>(entity);

                if (try_model != nullptr && try_animation_data != nullptr) {
                    auto& model = *try_model;
                    auto& animation_data = *try_animation_data;

                    auto& animations = model->get_animations();
                    i32 current_animation = static_cast<int>(animation_data.selected_animation);

                    ImGui::Text("Animation");
                    for (u32 j = 0; j < animations.size(); j++) {
                        float total_animation_time = animations[j].get_total_animation_time();
                        if ((int)j == current_animation) {
                            ImGui::Text("(Selected) Animation: %s, %f ticks", animations[j].m_name.c_str(), total_animation_time);

                        } else {
                            ImGui::Text("Animation: %s, %f ticks", animations[j].m_name.c_str(), total_animation_time);
                        }
                    }

                    if (ImGui::DragInt("Current Animation", &current_animation, 1.0F, 0, static_cast<int>(animations.size() - 1))) {
                        if (current_animation >= static_cast<i32>(animations.size())) {
                            current_animation = static_cast<i32>(animations.size() - 1);
                        }
                        animation_data.selected_animation = current_animation;
                        m_models_instance_draw_cache_needs_update = true;
                    }

                    ImGui::Checkbox("Pause Animation", &animation_data.data[animation_data.selected_animation]->m_paused);

                    float ticks_per_second = animations[current_animation].get_ticks_per_second();
                    if (ImGui::DragFloat("Ticks per second", &ticks_per_second)) {
                        animations[current_animation].set_ticks_per_second(ticks_per_second);
                    }
                }

                if (try_body_id != nullptr && try_motion_type != nullptr) {
                    auto& body_id = *try_body_id;
                    auto& motion_type = *try_motion_type;

                    ImGui::Text("Physics");
                    if (motion_type != JPH::EMotionType::Static) {
                        glm::vec3 cube_pos = vec3_to_vec3(m_physics_system->m_body_interface->GetPosition(body_id));
                        if (ImGui::DragFloat3("XYZ", &cube_pos.x, 1.0F, MIN_TRANSFORM, MAX_TRANSFORM)) {
                            m_physics_system->m_body_interface->SetPosition(
                                body_id,
                                vec3_to_vec3(cube_pos),
                                JPH::EActivation::Activate);
                        }

                        glm::quat cube_rot = quat_to_quat(m_physics_system->m_body_interface->GetRotation(body_id));
                        glm::vec3 euler_angles = glm::degrees(glm::eulerAngles(cube_rot));
                        if (ImGui::DragFloat3("Rotation: XYZ", &euler_angles.x, 1.0F, MIN_ROTATION, MAX_ROTATION)) {
                            m_physics_system->m_body_interface->SetRotation(
                                body_id,
                                quat_to_quat(glm::quat(glm::radians(euler_angles))),
                                JPH::EActivation::Activate);
                        }
                    }
                }

                if (try_transform != nullptr) {
                    auto& transform = *try_transform;

                    ImGui::Text("Transform");
                    glm::vec3 transform_pos = transform.get_position();
                    if (ImGui::DragFloat3("Position: XYZ", &transform_pos.x, 1.0F, MIN_TRANSFORM, MAX_TRANSFORM)) {
                        transform.set_position(transform_pos);
                    }

                    glm::vec3 transform_rot = transform.get_euler_angles();
                    if (ImGui::DragFloat3("Rotation: XYZ", &transform_rot.x, 1.0F, MIN_ROTATION, MAX_ROTATION)) {
                        transform.set_euler_angles(transform_rot);
                    }

                    glm::vec3 transform_scale = transform.get_scale();
                    if (ImGui::DragFloat3("Scale: XYZ", &transform_scale.x, 1.0F, MIN_TRANSFORM, MAX_TRANSFORM)) {
                        transform.set_scale(transform_scale);
                    }

                    if (ImGui::DragFloat("Scale: All", &transform_scale.x, 1.0F, MIN_TRANSFORM, MAX_TRANSFORM)) {
                        transform.set_scale(glm::vec3(transform_scale.x));
                    }
                }

                if (try_point != nullptr) {
                    auto& point = *try_point;

                    ImGui::Text("Point Light");
                    ImGui::DragFloat3("XYZ", &point.position.x, 1.0F, MIN_TRANSFORM, MAX_TRANSFORM);
                    ImGui::DragFloat3("RGB", &point.color.x, 10.0F, MIN_COLOR, MAX_COLOR);
                }

                if (try_directional != nullptr) {
                    auto& directional = *try_directional;

                    ImGui::Text("Directional Light");
                    ImGui::DragFloat3("XYZ", &directional.direction.x, 1.0F, -1.0F, 1.0F);
                    ImGui::DragFloat3("RGB", &directional.color.x, 10.0F, MIN_COLOR, MAX_COLOR);
                }

                if (try_spot != nullptr) {
                    auto& spot = *try_spot;

                    ImGui::Text("Spot Light");
                    ImGui::DragFloat3("Position XYZ", &spot.position.x, 1.0F, MIN_TRANSFORM, MAX_TRANSFORM);
                    ImGui::DragFloat3("Direction XYZ", &spot.direction.x, 1.0F, -1.0F, 1.0F);
                    ImGui::DragFloat3("RGB", &spot.color.x, 10.0F, MIN_COLOR, MAX_COLOR);
                    ImGui::DragFloat("inner_cutoff", &spot.inner_cutoff);
                    ImGui::DragFloat("outer_cutoff", &spot.outer_cutoff);
                }
            }

            ImGui::PopID();
            i++;
        }
    }
}

Renderer::Camera& Scene::get_camera()
{
    return m_app_data->m_camera;
}

const Utils::DeltaTime& Scene::get_clock()
{
    return m_clock;
}

void Scene::compile_shaders()
{
    if (!m_shaders_need_update) {
        return;
    }
    LOG_INFO("Compiling shaders");

    std::string no_defines;
    std::string bone_defines = Renderer::get_bone_defines();

    compile_pbr_shaders(no_defines, bone_defines);

    if (!m_shadowmap_shader.is_initialized()) {
        ShaderInfoData<2> shadowmap_shaders;
        get_shadow_pass_basic_shaders(shadowmap_shaders, no_defines, no_defines);
        m_shadowmap_shader.init(shadowmap_shaders.info.data(), shadowmap_shaders.info.size());
    }

    if (!m_shadowmap_shader_bones.is_initialized()) {
        ShaderInfoData<2> shadowmap_shaders;
        get_shadow_pass_basic_shaders(shadowmap_shaders, bone_defines, no_defines);
        m_shadowmap_shader_bones.init(shadowmap_shaders.info.data(), shadowmap_shaders.info.size());
    }

    if (!m_shadowmap_cubemap_shader.is_initialized()) {
        if constexpr (!Renderer::Light::Pbr::PointShadow::USE_GEOMETRY_SHADER) {
            ShaderInfoData<2> cubemap_shaders;
            get_shadow_pass_point_shaders(cubemap_shaders, no_defines, no_defines);
            m_shadowmap_cubemap_shader.init(cubemap_shaders.info.data(), cubemap_shaders.info.size());
        } else {
            ShaderInfoData<3> cubemap_shaders;
            get_shadow_pass_point_geometry_shaders(cubemap_shaders, no_defines, no_defines);
            m_shadowmap_cubemap_shader.init(cubemap_shaders.info.data(), cubemap_shaders.info.size());
        }
    }

    if (!m_shadowmap_cubemap_shader_bones.is_initialized()) {
        if constexpr (!Renderer::Light::Pbr::PointShadow::USE_GEOMETRY_SHADER) {
            ShaderInfoData<2> cubemap_shaders;
            get_shadow_pass_point_shaders(cubemap_shaders, bone_defines, no_defines);
            m_shadowmap_cubemap_shader_bones.init(cubemap_shaders.info.data(), cubemap_shaders.info.size());
        } else {
            ShaderInfoData<3> cubemap_shaders;
            get_shadow_pass_point_geometry_shaders(cubemap_shaders, bone_defines, no_defines);
            m_shadowmap_cubemap_shader_bones.init(cubemap_shaders.info.data(), cubemap_shaders.info.size());
        }
    }

    m_shaders_need_update = false;
}

void Scene::compile_pbr_shaders(const std::string& empty_defines, const std::string& bone_defines)
{
    std::string light_uniforms;
    std::string light_functions;

    u32 i = 0;
    auto directional_view = m_registry.view<Renderer::Light::Pbr::Directional>();
    for (auto [entity, light] : directional_view.each()) {
        light_uniforms += std::format("uniform DirectionalLight u_directional_light_{};\n", i);
        auto* shadow = m_registry.try_get<Renderer::Light::Pbr::DirectionalShadow>(entity);
        if (shadow != nullptr) {
            light_uniforms += std::format("uniform DirectionalLightShadow u_directional_light_shadow_{};\n", i);
            light_functions += std::format("lo += (1.0 - shadow_calculation_directional(u_directional_light_shadow_{}, bias)) * pbr_directional(u_directional_light_{}, albedo, roughness, metallic, normal, view, base_reflectivity);", i, i);
        } else {
            light_functions += std::format("lo += pbr_directional(u_directional_light_{}, albedo, roughness, metallic, normal, view, base_reflectivity);", i);
        }
        i++;
    }
    auto point_view = m_registry.view<Renderer::Light::Pbr::Point>();
    i = 0;
    for (auto [entity, light] : point_view.each()) {
        light_uniforms += std::format("uniform PointLight u_point_light_{};\n", i);

        auto* shadow = m_registry.try_get<Renderer::Light::Pbr::PointShadow>(entity);
        if (shadow != nullptr) {
            light_uniforms += std::format("uniform PointLightShadow u_point_light_shadow_{};\n", i);
            light_functions += std::format("lo += (1.0 - shadow_calculation_point(u_point_light_{}, u_point_light_shadow_{}, bias)) * pbr_point(u_point_light_{}, albedo, roughness, metallic, normal, view, base_reflectivity);", i, i, i);
        } else {
            light_functions += std::format("lo += pbr_point(u_point_light_{}, albedo, roughness, metallic, normal, view, base_reflectivity);", i);
        }
        i++;
    }
    auto spot_view = m_registry.view<Renderer::Light::Pbr::Spot>();
    i = 0;
    for (auto [entity, light] : spot_view.each()) {
        light_uniforms += std::format("uniform SpotLight u_spot_light_{};\n", i);

        auto* shadow = m_registry.try_get<Renderer::Light::Pbr::SpotShadow>(entity);
        if (shadow != nullptr) {
            light_uniforms += std::format("uniform SpotLightShadow u_spot_light_shadow_{};\n", i);
            light_functions += std::format("lo += (1.0 - shadow_calculation_spot(u_spot_light_shadow_{}, bias)) * pbr_spot(u_spot_light_{}, albedo, roughness, metallic, normal, view, base_reflectivity);", i, i, i);
        } else {
            light_functions += std::format("lo += pbr_spot(u_spot_light_{}, albedo, roughness, metallic, normal, view, base_reflectivity);", i);
        }
        i++;
    }

    ShaderInfoData<2> shader_source;
    ShaderInfoData<2> shader_source_bones;

    std::string fragment_defines = "#define RANDOM_SAMPLING\n";
    if (Renderer::Extensions::is_extension_supported("GL_ARB_bindless_texture")) {
        get_pbr_forward_pass_indirect(shader_source, light_uniforms, light_functions, empty_defines, fragment_defines);
        get_pbr_forward_pass_indirect(shader_source_bones, light_uniforms, light_functions, bone_defines, fragment_defines);
    } else {
        get_pbr_forward_pass_normal(shader_source, light_uniforms, light_functions, empty_defines, fragment_defines);
        get_pbr_forward_pass_normal(shader_source_bones, light_uniforms, light_functions, bone_defines, fragment_defines);
    }

    if (m_shader.is_initialized()) {
        m_shader.deinit();
    }
    m_shader.init(shader_source.info.data(), shader_source.info.size());

    if (m_shader_bones.is_initialized()) {
        m_shader_bones.deinit();
    }
    m_shader_bones.init(shader_source_bones.info.data(), shader_source_bones.info.size());
}
