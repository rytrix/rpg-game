#include "scene.hpp"

#include "../physics/helpers.hpp"

#include "../utils/file.hpp"

#include "glm/gtc/quaternion.hpp"
#include "shader_preprocessor.hpp"

#include "../renderer/random_sampling_texture.hpp"

#include "entity.hpp"
#include "transform.hpp"

namespace {

constexpr void get_pbr_forward_pass_indirect(ShaderInfoData<2>& out, const std::string& light_uniforms, const std::string& light_functions, const std::string& vert_defines, const std::string& frag_defines)
{
    std::vector<char> pbr_file = read_file<char>("res/shaders/forward_pass/pbr_combined.glsl");
    std::string_view pbr_file_view = { pbr_file.data(), pbr_file.size() };

    // Vertex Shader
    out.data.at(0) = std::format("#version 460 core\n#define SSBO0\n");
    out.data.at(0) += vert_defines;
    out.data.at(0) += get_lines_between_delims(pbr_file_view, "// Vertex Begin", "// Vertex End");

    // Fragment Shader
    out.data.at(1) += "#version 460 core\n#define BINDLESS_TEXTURES\n";
    out.data.at(1) += frag_defines;
    out.data.at(1) += get_lines_between_delims(pbr_file_view, "// Fragment Begin", "// Light Uniforms Begin");
    out.data.at(1) += light_uniforms;
    out.data.at(1) += get_lines_between_delims(pbr_file_view, "// Light Uniforms End", "// LO Functions Begin");
    out.data.at(1) += light_functions;
    out.data.at(1) += get_lines_between_delims(pbr_file_view, "// LO Functions End", "// Fragment End");

    out.populate_info();
}

constexpr void get_pbr_forward_pass_normal(ShaderInfoData<2>& out, const std::string& light_uniforms, const std::string& light_functions, const std::string& vert_defines, const std::string& frag_defines)
{
    std::vector<char> pbr_file = read_file<char>("res/shaders/forward_pass/pbr_combined.glsl");
    std::string_view pbr_file_view = { pbr_file.data(), pbr_file.size() };

    // Vertex Shader
    out.data.at(0) = std::format("#version 460 core\n#define SSBO0\n");
    out.data.at(0) += vert_defines;
    out.data.at(0) += get_lines_between_delims(pbr_file_view, "// Vertex Begin", "// Vertex End");

    // Fragment Shader
    out.data.at(1) += "#version 460 core\n#define UNIFORM_TEXTURES\n";
    out.data.at(1) += frag_defines;
    out.data.at(1) += get_lines_between_delims(pbr_file_view, "// Fragment Begin", "// Light Uniforms Begin");
    out.data.at(1) += light_uniforms;
    out.data.at(1) += get_lines_between_delims(pbr_file_view, "// Light Uniforms End", "// LO Functions Begin");
    out.data.at(1) += light_functions;
    out.data.at(1) += get_lines_between_delims(pbr_file_view, "// LO Functions End", "// Fragment End");

    out.populate_info();
}

constexpr void get_shadow_pass_basic_shaders(ShaderInfoData<2>& out, const std::string& vert_defines, const std::string& frag_defines)
{
    std::vector<char> shadow_file = read_file<char>("res/shaders/forward_pass/shadow_pass_basic.glsl");
    std::string_view shadow_file_view = { shadow_file.data(), shadow_file.size() };

    // Vertex Shader
    out.data.at(0) = "#version 460 core\n";
    out.data.at(0) += vert_defines;
    out.data.at(0) += get_lines_between_delims(shadow_file_view, "// Vertex Begin", "// Vertex End");

    // Fragment Shader
    out.data.at(1) += "#version 460 core\n";
    out.data.at(1) += frag_defines;
    out.data.at(1) += get_lines_between_delims(shadow_file_view, "// Fragment Begin", "// Fragment End");

    out.populate_info();
}

constexpr void get_shadow_pass_point_shaders(ShaderInfoData<2>& out, const std::string& vert_defines, const std::string& frag_defines)
{
    std::vector<char> shadow_file = read_file<char>("res/shaders/forward_pass/shadow_pass_point.glsl");
    std::string_view shadow_file_view = { shadow_file.data(), shadow_file.size() };

    // Vertex Shader
    out.data.at(0) = "#version 460 core\n";
    out.data.at(0) += vert_defines;
    out.data.at(0) += get_lines_between_delims(shadow_file_view, "// Vertex Begin", "// Vertex End");

    // Fragment Shader
    out.data.at(1) += "#version 460 core\n";
    out.data.at(1) += frag_defines;
    out.data.at(1) += get_lines_between_delims(shadow_file_view, "// Fragment Begin", "// Fragment End");

    out.populate_info();
}

constexpr void get_shadow_pass_point_geometry_shaders(ShaderInfoData<3>& out, const std::string& vert_defines, const std::string& frag_defines)
{
    std::vector<char> shadow_file = read_file<char>("res/shaders/forward_pass/shadow_pass_point_geometry.glsl");
    std::string_view shadow_file_view = { shadow_file.data(), shadow_file.size() };

    // Vertex Shader
    out.data.at(0) = "#version 460 core\n";
    out.data.at(0) += vert_defines;
    out.data.at(0) += get_lines_between_delims(shadow_file_view, "// Vertex Begin", "// Vertex End");

    // Geometry Shader
    out.data.at(1) = "#version 460 core\n";
    out.data.at(1) += get_lines_between_delims(shadow_file_view, "// Geometry Begin", "// Geometry End");

    // Fragment Shader
    out.data.at(2) += "#version 460 core\n";
    out.data.at(2) += frag_defines;
    out.data.at(2) += get_lines_between_delims(shadow_file_view, "// Fragment Begin", "// Fragment End");

    out.populate_info();
}

} // anonymous namespace

Scene::Scene(GlobalAppData* app_data)
    : m_app_data(app_data)
    , m_camera_speed(app_data->m_camera.get_speed())
    , m_random_sampling_texture(Renderer::RandomSamplingTexture::create(16, 8, 2, &app_data->m_texture_cache))
{
    m_physics_system = std::make_unique<Physics::System>();

    Renderer::SkyboxInfo skybox_info {};
    skybox_info.file = "res/skyboxes/Cubemap_Sky_14-512x512.png";
    m_skybox.init(skybox_info);

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

Entity Scene::create_entity()
{
    auto entity = m_registry.create();
    return { this, entity };
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

                    if (model_cached.m_model->get_mesh()->m_has_bones) {
                        auto& data = m_registry.get<Renderer::AnimationData>(entity);
                        model_cached.m_animation_data.emplace_back(&data);
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

    for (auto& model : m_models_instance_draw_cache) {
        for (auto* animation_data : model.m_animation_data) {
            auto* current_animation = animation_data->data[animation_data->selected_animation];
            Renderer::PerAnimationData* second_animation = nullptr;
            if (animation_data->second_animation != UINT32_MAX) {
                second_animation = animation_data->data[animation_data->second_animation];
            }
            if (animation_data->paused == false) {
                current_animation->m_animation_time += m_clock.delta_time<float>();
                if (second_animation != nullptr) {
                    second_animation->m_animation_time += m_clock.delta_time<float>();

                    const f32 BLEND_SPEED = 0.5F;
                    animation_data->blend_factor += m_clock.delta_time<float>() * BLEND_SPEED;

                    float factor = 1.0F - animation_data->blend_factor;
                    current_animation->update_transforms_blended(second_animation, factor);
                    if (animation_data->blend_factor >= 1.0F) {
                        animation_data->second_animation = UINT32_MAX;
                    }
                } else {
                    current_animation->update_transforms();
                }
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
            Renderer::Shader& shader = model.m_model->get_mesh()->m_has_bones ? m_shadowmap_cubemap_shader_bones : m_shadowmap_cubemap_shader;
            shadow.shadowmap_draw(shader, light, model.m_model);
        }
        shadow.shadowmap_end();
    }

    auto spot_shadow_view = m_registry.view<Renderer::Light::Pbr::Spot, Renderer::Light::Pbr::SpotShadow>();
    for (auto [entity, light, shadow] : spot_shadow_view.each()) {
        shadow.update(light);
        shadow.shadowmap_begin();
        for (auto& model : m_models_instance_draw_cache) {
            Renderer::Shader& shader = model.m_model->get_mesh()->m_has_bones ? m_shadowmap_shader_bones : m_shadowmap_shader;
            shadow.shadowmap_draw(shader, model.m_model);
        }
        shadow.shadowmap_end();
    }

    glCullFace(GL_BACK);
    glViewport(0, 0, m_app_data->m_window.get_width(), m_app_data->m_window.get_height());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (auto& model : m_models_instance_draw_cache) {
        Renderer::Shader& shader = model.m_model->get_mesh()->m_has_bones ? m_shader_bones : m_shader;
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

    m_skybox.draw(m_app_data->m_camera);

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

                    auto& animations = model->get_mesh()->m_animations;
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
                        animation_data.second_animation = animation_data.selected_animation;
                        animation_data.selected_animation = current_animation;
                        animation_data.blend_factor = 0.0F;
                        m_models_instance_draw_cache_needs_update = true;
                    }

                    ImGui::Checkbox("Pause Animation", &animation_data.paused);

                    float ticks_per_second = animations[current_animation].get_ticks_per_second();
                    if (ImGui::DragFloat("Ticks per second", &ticks_per_second)) {
                        animations[current_animation].set_ticks_per_second(ticks_per_second);
                    }
                }

                if (try_body_id != nullptr && try_motion_type != nullptr) {
                    auto& body_id = *try_body_id;
                    auto& motion_type = *try_motion_type;

                    if (motion_type != JPH::EMotionType::Static) {
                        ImGui::Text("Physics");
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
                    } else {
                        ImGui::Text("Physics - Static Object");
                    }
                }

                if (try_transform != nullptr && try_body_id == nullptr) {
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
