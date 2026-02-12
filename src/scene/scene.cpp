#include "scene.hpp"
#include "../physics/helpers.hpp"

namespace {

#include "scene_shaders.hpp"

}

Scene::Scene(Renderer::Window& window, Renderer::Camera& camera)
    : m_window(window)
    , m_camera(camera)
    , m_camera_speed(m_camera.get_speed())
{
    m_physics_system = std::make_unique<Physics::System>();

    init_pass();
    update();
}

Scene::~Scene()
{
    if (m_forward_pass) {
        delete m_forward;
    } else {
        delete m_deferred;
    }

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
        Renderer::Model& model = m_model_cache.get_or_create(entity_builder.m_model_path, entity_builder.m_model_path);
        m_registry.emplace<Renderer::Model*>(entity, &model);
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

    if (entity_builder.m_phong_directional_info != nullptr) {
        m_registry.emplace<Renderer::Light::Phong::Directional>(entity, *entity_builder.m_phong_directional_info);
        m_shaders_need_update = true;
    }

    if (entity_builder.m_phong_point_info != nullptr) {
        m_registry.emplace<Renderer::Light::Phong::Point>(entity, *entity_builder.m_phong_point_info);
        m_shaders_need_update = true;
    }

    if (entity_builder.m_pbr_point != nullptr) {
        m_registry.emplace<Renderer::Light::Pbr::Point>(entity, *entity_builder.m_pbr_point);
        m_shaders_need_update = true;
    }

    if (entity_builder.m_pbr_directional != nullptr) {
        m_registry.emplace<Renderer::Light::Pbr::Directional>(entity, *entity_builder.m_pbr_directional);
        m_shaders_need_update = true;
    }

    if (entity_builder.m_pbr_spot != nullptr) {
        m_registry.emplace<Renderer::Light::Pbr::Spot>(entity, *entity_builder.m_pbr_spot);
        m_shaders_need_update = true;
    }

    m_registry.emplace<glm::mat4>(entity, entity_builder.m_model_matrix);
}

void Scene::optimize()
{
    m_physics_system->optimize();
    m_physics_needs_optimize = false;
}

void Scene::update()
{
    m_clock.update();

    if (m_models_instance_draw_cache_needs_update) {
        auto model_view = m_registry.view<glm::mat4, Renderer::Model*>();

        m_models_instance_draw_cache.clear();
        for (auto [entity, model_matrix, model] : model_view.each()) {
            for (auto& model_cached : m_models_instance_draw_cache) {
                if (model == model_cached.model) {
                    model_cached.model_matrices.emplace_back(model_matrix);
                    goto end_model_matrix_label;
                }
            }
            m_models_instance_draw_cache.emplace_back(model, model_matrix);
        end_model_matrix_label:
        }

        LOG_INFO("Updated scene instanced draw cache");
        m_models_instance_draw_cache_needs_update = false;
    } else {
        auto model_view = m_registry.view<glm::mat4, Renderer::Model*>();

        for (auto& model : m_models_instance_draw_cache) {
            model.model_matrices.clear();
        }
        for (auto [entity, model_matrix, model] : model_view.each()) {
            for (usize j = 0; j < m_models_instance_draw_cache.size(); j++) {
                if (model == m_models_instance_draw_cache[j].model) {
                    m_models_instance_draw_cache[j].model_matrices.emplace_back(model_matrix);
                }
            }
        }
    }

    if (m_deferred != nullptr) {
        if (m_window.get_width() != m_deferred->m_gpass_width || m_window.get_height() != m_deferred->m_gpass_height) {
            m_deferred->m_gpass_width = m_window.get_width();
            m_deferred->m_gpass_height = m_window.get_height();
            m_deferred->m_gpass.reinit(m_deferred->m_gpass_width, m_deferred->m_gpass_height);
        }
    }
    compile_shaders();
}

void Scene::physics()
{
    m_physics_system->update(m_clock.delta_time());

    auto view = m_registry.view<glm::mat4, JPH::BodyID, JPH::EMotionType>();

    for (auto [entity, model, body, motion] : view.each()) {
        if (motion != JPH::EMotionType::Static) {
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

void Scene::instance_draw_internal(Renderer::ShaderProgram& shader, bool shadowmap)
{
    for (auto& model : m_models_instance_draw_cache) {
        if (shadowmap) {
            model.model->draw_untextured(shader, model.model_matrices);
        } else {
            model.model->draw(shader, model.model_matrices);
        }
    }
}

void Scene::draw()
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    m_camera.update();

    auto phong_directional_view = m_registry.view<Renderer::Light::Phong::Directional>();
    for (auto [entity, light] : phong_directional_view.each()) {
        if (light.has_shadowmap()) {
            light.shadowmap_draw(m_shadowmap_shader, [&]() {
                instance_draw_internal(m_shadowmap_shader, true);
            });
        }
    }

    auto phong_point_view = m_registry.view<Renderer::Light::Phong::Point>();
    for (auto [entity, light] : phong_point_view.each()) {
        if (light.has_shadowmap()) {
            light.shadowmap_draw(m_shadowmap_cubemap_shader, [&]() {
                instance_draw_internal(m_shadowmap_cubemap_shader, true);
            });
        }
    }

    if (m_forward_pass) {
        glViewport(0, 0, m_window.get_width(), m_window.get_height());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_forward->m_shader.bind();
        m_forward->m_shader.set_mat4("proj", m_camera.get_proj());
        m_forward->m_shader.set_mat4("view", m_camera.get_view());
        m_forward->m_shader.set_vec3("view_position", m_camera.get_pos());

        auto pbr_point_view = m_registry.view<Renderer::Light::Pbr::Point>();
        auto pbr_directional_view = m_registry.view<Renderer::Light::Pbr::Directional>();
        auto pbr_spot_view = m_registry.view<Renderer::Light::Pbr::Spot>();
        u32 i = 0;
        for (auto [entity, light] : pbr_directional_view.each()) {
            light.set_uniforms(m_forward->m_shader, std::format("u_directional_light_{}", i).c_str());
            i++;
        }
        i = 0;
        for (auto [entity, light] : pbr_point_view.each()) {
            light.set_uniforms(m_forward->m_shader, std::format("u_point_light_{}", i).c_str());
            i++;
        }
        i = 0;
        for (auto [entity, light] : pbr_spot_view.each()) {
            light.set_uniforms(m_forward->m_shader, std::format("u_spot_light_{}", i).c_str());
            i++;
        }

        instance_draw_internal(m_forward->m_shader, false);
    } else {
        // Geometry pass
        m_deferred->m_gpass.bind();
        glViewport(0, 0, m_window.get_width(), m_window.get_height());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_deferred->m_gpass_shader.bind();
        m_deferred->m_gpass_shader.set_mat4("proj", m_camera.get_proj());
        m_deferred->m_gpass_shader.set_mat4("view", m_camera.get_view());

        instance_draw_internal(m_deferred->m_gpass_shader, false);

        m_deferred->m_gpass.blit_depth_buffer();
        m_deferred->m_gpass.unbind();

        // Lighting pass
        glClear(GL_COLOR_BUFFER_BIT);
        m_deferred->m_lpass_shader.bind();

        m_deferred->m_gpass.set_uniforms(m_deferred->m_lpass_shader);
        m_deferred->m_lpass_shader.set_vec3("view_position", m_camera.get_pos());

        u32 i = 0;
        for (auto [entity, light] : phong_directional_view.each()) {
            light.set_uniforms(m_deferred->m_lpass_shader, std::format("u_directional_light_{}", i).c_str());
            i++;
        }
        i = 0;
        for (auto [entity, light] : phong_point_view.each()) {
            light.set_uniforms(m_deferred->m_lpass_shader, std::format("u_point_light_{}", i).c_str());
            i++;
        }
        m_deferred->m_lpass.draw();
    }
    Renderer::Texture::reset_texture_units();
}

void Scene::set_pass(bool forward)
{
    if (m_forward_pass != forward) {
        m_forward_pass = forward;
        init_pass();
        update();
    }
}

void Scene::draw_debug_imgui()
{
    if (ImGui::Button("Reload shaders")) {
        m_shaders_need_update = true;
    }

    if (ImGui::DragFloat("Camera Speed", &m_camera_speed, 0.1F, 1.0F, 20.0F)) {
        m_camera.set_speed(m_camera_speed);
    }

    constexpr float MAX_TRANSFORM = 32.0F;
    constexpr float MIN_TRANSFORM = -32.0F;

    constexpr float MAX_COLOR = 3000.0F;
    constexpr float MIN_COLOR = 0.0F;

    usize i = 0;
    if (ImGui::CollapsingHeader("Physics Objects")) {
        auto view = m_registry.view<glm::mat4, JPH::BodyID, JPH::EMotionType>();
        for (auto [entity, model_matrix, body, motion_type] : view.each()) {
            if (motion_type != JPH::EMotionType::Static) {
                ImGui::PushID(i);
                const char** name_check = m_registry.try_get<const char*>(entity);
                const char* name;
                if (name_check == nullptr) {
                    name = "no_name";
                } else {
                    name = *name_check;
                }

                if (ImGui::CollapsingHeader(std::format("{}_e{}", name, i).c_str())) {
                    glm::vec4& cube_pos = model_matrix[3];
                    ImGui::DragFloat3("XYZ", &cube_pos.x, 1.0F, MIN_TRANSFORM, MAX_TRANSFORM);
                    m_physics_system->m_body_interface->SetPosition(
                        body,
                        vec3_to_vec3(cube_pos),
                        JPH::EActivation::Activate);
                }
                ImGui::PopID();
                i++;
            }
        }
    }

    if (ImGui::CollapsingHeader("Lights")) {
        auto point_view = m_registry.view<Renderer::Light::Pbr::Point>();
        auto directional_view = m_registry.view<Renderer::Light::Pbr::Directional>();
        auto spot_view = m_registry.view<Renderer::Light::Pbr::Spot>();

        for (auto [entity, light] : point_view.each()) {
            ImGui::PushID(i);
            const char** name_check = m_registry.try_get<const char*>(entity);
            const char* name;
            if (name_check == nullptr) {
                name = "no_name";
            } else {
                name = *name_check;
            }

            if (ImGui::CollapsingHeader(std::format("{}_PL{}", name, i).c_str())) {
                ImGui::DragFloat3("XYZ", &light.position.x, 1.0F, MIN_TRANSFORM, MAX_TRANSFORM);
                ImGui::DragFloat3("RGB", &light.color.x, 10.0F, MIN_COLOR, MAX_COLOR);
            }

            ImGui::PopID();
            i++;
        }

        for (auto [entity, light] : directional_view.each()) {
            ImGui::PushID(i);
            const char** name_check = m_registry.try_get<const char*>(entity);
            const char* name;
            if (name_check == nullptr) {
                name = "no_name";
            } else {
                name = *name_check;
            }

            if (ImGui::CollapsingHeader(std::format("{}_DL{}", name, i).c_str())) {
                ImGui::DragFloat3("XYZ", &light.direction.x, 1.0F, -1.0F, 1.0F);
                ImGui::DragFloat3("RGB", &light.color.x, 10.0F, MIN_COLOR, MAX_COLOR);
            }
            ImGui::PopID();
            i++;
        }

        for (auto [entity, light] : spot_view.each()) {
            ImGui::PushID(i);
            const char** name_check = m_registry.try_get<const char*>(entity);
            const char* name;
            if (name_check == nullptr) {
                name = "no_name";
            } else {
                name = *name_check;
            }

            if (ImGui::CollapsingHeader(std::format("{}_SL{}", name, i).c_str())) {
                ImGui::DragFloat3("Position XYZ", &light.position.x, 1.0F, MIN_TRANSFORM, MAX_TRANSFORM);
                ImGui::DragFloat3("Direction XYZ", &light.direction.x, 1.0F, -1.0F, 1.0F);
                ImGui::DragFloat3("RGB", &light.color.x, 10.0F, MIN_COLOR, MAX_COLOR);
                ImGui::DragFloat("inner_cutoff", &light.inner_cutoff);
                ImGui::DragFloat("outer_cutoff", &light.outer_cutoff);
            }
            ImGui::PopID();
            i++;
        }
    }
}

Renderer::Camera& Scene::get_camera()
{
    return m_camera;
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

    compile_pbr_shaders();

    if (!m_shadowmap_shader.is_initialized()) {
        auto shadowmap_info = Renderer::ShadowMap::get_shader_info();
        m_shadowmap_shader.init(shadowmap_info.data(), shadowmap_info.size());
    }

    if (!m_shadowmap_cubemap_shader.is_initialized()) {
        auto shadowmap_cubemap_info = Renderer::ShadowMap::get_shader_info_cubemap();
        m_shadowmap_cubemap_shader.init(shadowmap_cubemap_info.data(), shadowmap_cubemap_info.size());
    }

    m_shaders_need_update = false;
}

void Scene::compile_pbr_shaders()
{
    std::string light_uniforms;
    std::string light_functions;

    u32 i = 0;
    auto directional_view = m_registry.view<Renderer::Light::Pbr::Directional>();
    for (auto [entity, light] : directional_view.each()) {
        light_uniforms += std::format("uniform DirectionalLight u_directional_light_{};\n", i);
        light_functions += std::format("lo += pbr_directional(u_directional_light_{}, albedo, roughness, metallic, normal, view);", i);
        i++;
    }
    auto point_view = m_registry.view<Renderer::Light::Pbr::Point>();
    i = 0;
    for (auto [entity, light] : point_view.each()) {
        light_uniforms += std::format("uniform PointLight u_point_light_{};\n", i);
        light_functions += std::format("lo += pbr_point(u_point_light_{}, albedo, roughness, metallic, normal, view);", i);
        i++;
    }
    auto spot_view = m_registry.view<Renderer::Light::Pbr::Spot>();
    i = 0;
    for (auto [entity, light] : spot_view.each()) {
        light_uniforms += std::format("uniform SpotLight u_spot_light_{};\n", i);
        light_functions += std::format("lo += pbr_spot(u_spot_light_{}, albedo, roughness, metallic, normal, view);", i);
        i++;
    }

    if (m_forward_pass) {
        std::string shader_source_frag;
        const char* shader_source_vert;
        if (Renderer::Extensions::is_extension_supported("GL_ARB_bindless_texture")) {
            shader_source_frag = get_pbr_forward_pass_indirect(light_uniforms, light_functions);
            shader_source_vert = "res/forward_pass/model_indirect.glsl.vert";
        } else {
            shader_source_frag = get_pbr_forward_pass_normal(light_uniforms, light_functions);
            // shader_source_frag = "res/forward_pass/pbr_normal.glsl.frag";
            shader_source_vert = "res/forward_pass/model_normal.glsl.vert";
        }

        std::array<Renderer::ShaderInfo, 2>
            shader_info = {
                Renderer::ShaderInfo {
                    .is_file = true,
                    .shader = shader_source_vert,
                    .type = GL_VERTEX_SHADER,
                },
                Renderer::ShaderInfo {
                    .is_file = false,
                    .shader = shader_source_frag.c_str(),
                    .type = GL_FRAGMENT_SHADER,
                },
            };
        if (m_forward->m_shader.is_initialized()) {
            m_forward->m_shader.~ShaderProgram();
        }
        m_forward->m_shader.init(shader_info.data(), shader_info.size());
    }
}

void Scene::compile_phong_shaders()
{
    std::string light_uniforms;
    std::string light_functions;
    auto directional_view = m_registry.view<Renderer::Light::Phong::Directional>();

    u32 i = 0;
    for (auto [entity, light] : directional_view.each()) {
        if (light.has_shadowmap()) {
            light_uniforms += std::format("uniform DirectionalLightShadow u_directional_light_{};\n", i);
            light_functions += std::format("FragColor += vec4(directional_light_shadow(u_directional_light_{}, Albedo, Specular, Normal, view_position, FragPos, 32.0), 1.0);\n", i);
        } else {
            light_uniforms += std::format("uniform DirectionalLight u_directional_light_{};\n", i);
            light_functions += std::format("FragColor += vec4(directional_light(u_directional_light_{}, Albedo, Specular, Normal, view_position, FragPos, 32.0), 1.0);\n", i);
        }
        i++;
    }
    auto point_view = m_registry.view<Renderer::Light::Phong::Point>();
    i = 0;
    for (auto [entity, light] : point_view.each()) {
        if (light.has_shadowmap()) {
            light_uniforms += std::format("uniform PointLightShadow u_point_light_{};\n", i);
            light_functions += std::format("FragColor += vec4(point_light_shadow(u_point_light_{}, Albedo, Specular, Normal, view_position, FragPos, 32.0), 1.0);\n", i);
        } else {
            light_uniforms += std::format("uniform PointLight u_point_light_{};\n", i);
            light_functions += std::format("FragColor += vec4(point_light(u_point_light_{}, Albedo, Specular, Normal, view_position, FragPos, 32.0), 1.0);\n", i);
        }
        i++;
    }

    if (m_forward_pass) {
        std::string shader_source_frag;
        const char* shader_source_vert;
        if (Renderer::Extensions::is_extension_supported("GL_ARB_bindless_texture")) {
            shader_source_frag = get_phong_forward_pass_indirect(light_uniforms, light_functions);
            shader_source_vert = "res/forward_pass/model_indirect.glsl.vert";
        } else {
            shader_source_frag = get_phong_forward_pass_normal(light_uniforms, light_functions);
            // shader_source_frag = "res/forward_pass/pbr_normal.glsl.frag";
            shader_source_vert = "res/forward_pass/model_normal.glsl.vert";
        }

        std::array<Renderer::ShaderInfo, 2>
            shader_info = {
                Renderer::ShaderInfo {
                    .is_file = true,
                    .shader = shader_source_vert,
                    .type = GL_VERTEX_SHADER,
                },
                Renderer::ShaderInfo {
                    .is_file = true,
                    .shader = shader_source_frag.c_str(),
                    .type = GL_FRAGMENT_SHADER,
                },
            };
        if (m_forward->m_shader.is_initialized()) {
            m_forward->m_shader.~ShaderProgram();
        }
        m_forward->m_shader.init(shader_info.data(), shader_info.size());
    } else {
        std::array<Renderer::ShaderInfo, 2> shader_info;

        if (Renderer::Extensions::is_extension_supported("GL_ARB_bindless_texture")) {
            shader_info = {
                Renderer::ShaderInfo {
                    .is_file = true,
                    .shader = "res/deferred_shading/g_pass_indirect.glsl.vert",
                    .type = GL_VERTEX_SHADER,
                },
                Renderer::ShaderInfo {
                    .is_file = true,
                    .shader = "res/deferred_shading/g_pass_indirect.glsl.frag",
                    .type = GL_FRAGMENT_SHADER,
                },
            };
        } else {
            shader_info = {
                Renderer::ShaderInfo {
                    .is_file = true,
                    .shader = "res/deferred_shading/g_pass_normal.glsl.vert",
                    .type = GL_VERTEX_SHADER,
                },
                Renderer::ShaderInfo {
                    .is_file = true,
                    .shader = "res/deferred_shading/g_pass_normal.glsl.frag",
                    .type = GL_FRAGMENT_SHADER,
                },
            };
        }
        if (m_deferred->m_gpass_shader.is_initialized()) {
            m_deferred->m_gpass_shader.~ShaderProgram();
        }
        m_deferred->m_gpass_shader.init(shader_info.data(), shader_info.size());

        std::string shader_source_frag = get_deferred_pass(light_uniforms, light_functions);

        shader_info = {
            Renderer::ShaderInfo {
                .is_file = true,
                .shader = "res/deferred_shading/l_pass.glsl.vert",
                .type = GL_VERTEX_SHADER,
            },
            Renderer::ShaderInfo {
                .is_file = false,
                .shader = shader_source_frag.c_str(),
                .type = GL_FRAGMENT_SHADER,
            },
        };
        if (m_deferred->m_lpass_shader.is_initialized()) {
            m_deferred->m_lpass_shader.~ShaderProgram();
        }
        m_deferred->m_lpass_shader.init(shader_info.data(), shader_info.size());
    }
}

void Scene::init_pass()
{
    if (m_forward != nullptr) {
        delete m_forward;
        m_forward = nullptr;
        LOG_INFO("Deleted forward pass");
    }
    if (m_deferred != nullptr) {
        delete m_deferred;
        m_deferred = nullptr;
        LOG_INFO("Deleted deferred pass");
    }

    if (m_forward_pass) {
        m_forward = new ForwardPass {};
        LOG_INFO("Created forward pass");
        glEnable(GL_MULTISAMPLE);
    } else {
        m_deferred = new DeferedPass {};
        m_deferred->m_gpass_width = m_window.get_width();
        m_deferred->m_gpass_height = m_window.get_height();
        m_deferred->m_gpass.init(m_deferred->m_gpass_width, m_deferred->m_gpass_height);
        m_deferred->m_lpass.init();
        LOG_INFO("Created deferred pass");
        glDisable(GL_MULTISAMPLE);
    }

    m_shaders_need_update = true;
}