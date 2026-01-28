#include "scene.hpp"

namespace {

#include "scene_shaders.hpp"

}

Scene::Scene()
{
    compile_shaders();
}

void Scene::compile_shaders()
{
    // TODO: fix the shader source
    std::string shader_source = std::format(
        R"(
        #version 460 core
        out vec4 FragColor;
        {} // Lighting code
        {} // Deferred in/uniforms
        {} // Light uniforms
        void main() {{
        vec3 FragPos = texture(gPosition, TexCoords).xyz;
        vec3 Normal = texture(gNormal, TexCoords).xyz;
        vec3 Albedo = texture(gAlbedoSpec, TexCoords).rgb;
        float Specular = texture(gAlbedoSpec, TexCoords).a;
        FragColor = vec4(0.0);
        {}
        }})",
        LIGHTING_SHADER_CODE,
        BOILERPLATE_SHADER_CODE_DEFERRED,
        "light uniforms",
        "light function calls");

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
            .is_file = true,
            .shader = "res/deferred_shading/l_pass.glsl.frag",
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
}

void Scene::add_entity(EntityBuilder& entity_builder)
{
    auto entity = m_registry.create();
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
    }

    if (entity_builder.m_point_info != nullptr) {
        m_registry.emplace<Renderer::Light::Point>(entity, *entity_builder.m_point_info);
    }

    m_registry.emplace<glm::mat4>(entity, entity_builder.m_model_matrix);
}