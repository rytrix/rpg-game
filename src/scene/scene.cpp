#include "scene.hpp"
#include "../physics/helpers.hpp"

namespace {

#include "scene_shaders.hpp"

}

Scene::Scene(Renderer::Window& window)
    : m_window(window)
    , m_gpass_width(m_window.get_width())
    , m_gpass_height(m_window.get_height())
{
    m_physics_system = std::make_unique<Physics::System>();

    m_camera.init(90.0F, 0.1F, 1000.0F, m_window.get_aspect_ratio(), { -2.0F, 1.5F, 4.0F });
    m_camera.set_speed(5.0F);

    m_gpass.init(m_gpass_width, m_gpass_height);
    m_lpass.init();

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

void Scene::add_entity(EntityBuilder& entity_builder)
{
    auto entity = m_registry.create();
    if (entity_builder.m_name != nullptr) {
        m_registry.emplace<const char*>(entity, entity_builder.m_name);
    }

    if (entity_builder.m_model_path != nullptr) {
        m_registry.emplace<Renderer::Model>(entity, entity_builder.m_model_path);
        // TODO: Decide if I want physics objects without models someday
        if (entity_builder.m_create_body != nullptr) {
            auto physics_info
                = entity_builder.m_create_body(m_physics_system.get(), &m_registry.get<Renderer::Model>(entity));
            m_registry.emplace<JPH::BodyID>(entity, physics_info.first);
            m_registry.emplace<JPH::EMotionType>(entity, physics_info.second);
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

void Scene::update()
{
    m_clock.update();
    if (m_window.get_width() != m_gpass_width || m_window.get_height() != m_gpass_height) {
        m_gpass_width = m_window.get_width();
        m_gpass_height = m_window.get_height();
        m_gpass.reinit(m_gpass_width, m_gpass_height);
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

    auto model_view = m_registry.view<glm::mat4, Renderer::Model>();

    auto directional_view = m_registry.view<Renderer::Light::Directional>();
    for (auto [entity, light] : directional_view.each()) {
        if (light.has_shadowmap()) {
            light.shadowmap_draw(m_shadowmap_shader, [&]() {
                for (auto [entity2, model_matrix, model] : model_view.each()) {
                    model.draw_untextured(m_shadowmap_shader, model_matrix);
                }
            });
        }
    }

    auto point_view = m_registry.view<Renderer::Light::Point>();
    for (auto [entity, light] : point_view.each()) {
        if (light.has_shadowmap()) {
            light.shadowmap_draw(m_shadowmap_cubemap_shader, [&]() {
                for (auto [entity2, model_matrix, model] : model_view.each()) {
                    model.draw_untextured(m_shadowmap_cubemap_shader, model_matrix);
                }
            });
        }
    }

    // Geometry pass
    m_gpass.bind();
    glViewport(0, 0, m_window.get_width(), m_window.get_height());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_gpass_shader.bind();
    m_gpass_shader.set_mat4("proj", m_camera.get_proj());
    m_gpass_shader.set_mat4("view", m_camera.get_view());

    for (auto [entity, model_matrix, model] : model_view.each()) {
        model.draw(m_gpass_shader, model_matrix);
    }

    m_gpass.blit_depth_buffer();
    m_gpass.unbind();

    // Lighting pass
    glClear(GL_COLOR_BUFFER_BIT);
    m_lpass_shader.bind();

    m_gpass.set_uniforms(m_lpass_shader);
    m_lpass_shader.set_vec3("view_position", m_camera.get_pos());

    u32 i = 0;
    for (auto [entity, light] : directional_view.each()) {
        light.set_uniforms(m_lpass_shader, std::format("u_directional_light_{}", i).c_str());
        i++;
    }
    i = 0;
    for (auto [entity, light] : point_view.each()) {
        light.set_uniforms(m_lpass_shader, std::format("u_point_light_{}", i).c_str());
        i++;
    }
    m_lpass.draw();

    Renderer::Texture::reset_texture_units();
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

    std::string shader_source = std::format(
        R"(
#version 460 core
out vec4 FragColor;

// Lighting code
{}

// Deferred in/uniforms
{}

// Light uniforms
{}

void main() {{
    vec3 FragPos = texture(gPosition, TexCoords).xyz;
    vec3 Normal = texture(gNormal, TexCoords).xyz;
    vec3 Albedo = texture(gAlbedoSpec, TexCoords).rgb;
    float Specular = texture(gAlbedoSpec, TexCoords).a;
    FragColor = vec4(0.0);
    {}
    // FragColor = vec4(Albedo, 1.0);
}})",
        LIGHTING_SHADER_CODE,
        BOILERPLATE_SHADER_CODE_DEFERRED,
        light_uniforms,
        light_functions);

    LOG_INFO(std::format("shader_source:\n {}", shader_source));

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
    if (m_gpass_shader.is_initialized()) {
        m_gpass_shader.~ShaderProgram();
    }
    m_gpass_shader.init(shader_info.data(), shader_info.size());

    shader_info = {
        Renderer::ShaderInfo {
            .is_file = true,
            .shader = "res/deferred_shading/l_pass.glsl.vert",
            .type = GL_VERTEX_SHADER,
        },
        // Todo swap to dynamic shaders
        Renderer::ShaderInfo {
            // .is_file = true,
            // .shader = "res/deferred_shading/l_pass.glsl.frag",
            .is_file = false,
            .shader = shader_source.c_str(),
            .type = GL_FRAGMENT_SHADER,
        },
    };
    if (m_lpass_shader.is_initialized()) {
        m_lpass_shader.~ShaderProgram();
    }
    m_lpass_shader.init(shader_info.data(), shader_info.size());

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