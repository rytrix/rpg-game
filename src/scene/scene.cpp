#include "scene.hpp"
#include "../physics/helpers.hpp"

namespace {

#include "scene_shaders.hpp"

}

Scene::Scene(Renderer::Window& window)
    : m_window(window)
{
    m_physics_system = std::make_unique<Physics::System>();

    m_camera.init(90.0F, 0.1F, 1000.0F, m_window.get_aspect_ratio(), { -2.0F, 1.5F, 4.0F });
    m_camera.set_speed(5.0F);

    init_pass();
    update();
}

Scene::~Scene()
{
    if (m_forward_pass) {
        delete m_forward;
    } else {
        delete m_deffered;
    }

    auto view = m_registry.view<JPH::BodyID>();
    for (auto [entity, body] : view.each()) {
        m_physics_system->m_body_interface->RemoveBody(body);
        m_physics_system->m_body_interface->DestroyBody(body);
    }
}

void Scene::add_entity(EntityBuilder& entity_builder)
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
    }

    if (entity_builder.m_directional_info != nullptr) {
        m_registry.emplace<Renderer::Light::Directional>(entity, *entity_builder.m_directional_info);
        m_shaders_need_update = true;
    }

    if (entity_builder.m_point_info != nullptr) {
        m_registry.emplace<Renderer::Light::Point>(entity, *entity_builder.m_point_info);
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
    if (m_deffered != nullptr) {
        if (m_window.get_width() != m_deffered->m_gpass_width || m_window.get_height() != m_deffered->m_gpass_height) {
            m_deffered->m_gpass_width = m_window.get_width();
            m_deffered->m_gpass_height = m_window.get_height();
            m_deffered->m_gpass.reinit(m_deffered->m_gpass_width, m_deffered->m_gpass_height);
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
        }
    }
}

void Scene::draw()
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    m_camera.update();

    auto model_view = m_registry.view<glm::mat4, Renderer::Model*>();

    auto directional_view = m_registry.view<Renderer::Light::Directional>();
    for (auto [entity, light] : directional_view.each()) {
        if (light.has_shadowmap()) {
            light.shadowmap_draw(m_shadowmap_shader, [&]() {
                for (auto [entity2, model_matrix, model] : model_view.each()) {
                    model->draw_untextured(m_shadowmap_shader, model_matrix);
                }
            });
        }
    }

    auto point_view = m_registry.view<Renderer::Light::Point>();
    for (auto [entity, light] : point_view.each()) {
        if (light.has_shadowmap()) {
            light.shadowmap_draw(m_shadowmap_cubemap_shader, [&]() {
                for (auto [entity2, model_matrix, model] : model_view.each()) {
                    model->draw_untextured(m_shadowmap_cubemap_shader, model_matrix);
                }
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

        u32 i = 0;
        for (auto [entity, light] : directional_view.each()) {
            light.set_uniforms(m_forward->m_shader, std::format("u_directional_light_{}", i).c_str());
            i++;
        }
        i = 0;
        for (auto [entity, light] : point_view.each()) {
            light.set_uniforms(m_forward->m_shader, std::format("u_point_light_{}", i).c_str());
            i++;
        }

        for (auto [entity, model_matrix, model] : model_view.each()) {
            model->draw(m_forward->m_shader, model_matrix);
        }
    } else {
        // Geometry pass
        m_deffered->m_gpass.bind();
        glViewport(0, 0, m_window.get_width(), m_window.get_height());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_deffered->m_gpass_shader.bind();
        m_deffered->m_gpass_shader.set_mat4("proj", m_camera.get_proj());
        m_deffered->m_gpass_shader.set_mat4("view", m_camera.get_view());

        for (auto [entity, model_matrix, model] : model_view.each()) {
            model->draw(m_deffered->m_gpass_shader, model_matrix);
        }

        // m_gpass.blit_depth_buffer();
        m_deffered->m_gpass.unbind();

        // Lighting pass
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_deffered->m_lpass_shader.bind();

        m_deffered->m_gpass.set_uniforms(m_deffered->m_lpass_shader);
        m_deffered->m_lpass_shader.set_vec3("view_position", m_camera.get_pos());

        u32 i = 0;
        for (auto [entity, light] : directional_view.each()) {
            light.set_uniforms(m_deffered->m_lpass_shader, std::format("u_directional_light_{}", i).c_str());
            i++;
        }
        i = 0;
        for (auto [entity, light] : point_view.each()) {
            light.set_uniforms(m_deffered->m_lpass_shader, std::format("u_point_light_{}", i).c_str());
            i++;
        }
        m_deffered->m_lpass.draw();
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
    auto view = m_registry.view<glm::mat4, JPH::BodyID, JPH::EMotionType>();

    u32 i = 0;
    for (auto [entity, model_matrix, body, motion_type] : view.each()) {
        if (motion_type != JPH::EMotionType::Static) {
            const char* name = m_registry.get<const char*>(entity);
            if (name == nullptr) {
                name = "no_name";
            }

            if (ImGui::CollapsingHeader(std::format("{}_e{}", name, i).c_str())) {
                glm::vec4& cube_pos = model_matrix[3];
                ImGui::DragFloat3("XYZ", &cube_pos.x, 1.0F, -8.0f, 8.0f);
                m_physics_system->m_body_interface->SetPosition(
                    body,
                    vec3_to_vec3(cube_pos),
                    JPH::EActivation::Activate);
            }
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

    std::string light_uniforms;
    std::string light_functions;
    auto directional_view = m_registry.view<Renderer::Light::Directional>();

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
    auto point_view = m_registry.view<Renderer::Light::Point>();
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
            shader_source_frag = get_forward_pass_indirect(light_uniforms, light_functions);
            shader_source_vert = "res/forward_pass/model_indirect.glsl.vert";
        } else {
            shader_source_frag = get_forward_pass_normal(light_uniforms, light_functions);
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
        if (m_deffered->m_gpass_shader.is_initialized()) {
            m_deffered->m_gpass_shader.~ShaderProgram();
        }
        m_deffered->m_gpass_shader.init(shader_info.data(), shader_info.size());

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
        if (m_deffered->m_lpass_shader.is_initialized()) {
            m_deffered->m_lpass_shader.~ShaderProgram();
        }
        m_deffered->m_lpass_shader.init(shader_info.data(), shader_info.size());
    }

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

void Scene::init_pass()
{
    if (m_forward != nullptr) {
        delete m_forward;
        m_forward = nullptr;
        LOG_INFO("Deleted forward pass");
    }
    if (m_deffered != nullptr) {
        delete m_deffered;
        m_deffered = nullptr;
        LOG_INFO("Deleted deferred pass");
    }

    if (m_forward_pass) {
        m_forward = new ForwardPass {};
        LOG_INFO("Created forward pass");
        glEnable(GL_MULTISAMPLE);
    } else {
        m_deffered = new DeferedPass {};
        m_deffered->m_gpass_width = m_window.get_width();
        m_deffered->m_gpass_height = m_window.get_height();
        m_deffered->m_gpass.init(m_deffered->m_gpass_width, m_deffered->m_gpass_height);
        m_deffered->m_lpass.init();
        LOG_INFO("Created deferred pass");
        glDisable(GL_MULTISAMPLE);
    }

    m_shaders_need_update = true;
}