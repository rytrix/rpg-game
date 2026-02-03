#pragma once

#include "../physics/engine.hpp"
#include "../utils/cache.hpp"
#include "../utils/deltatime.hpp"

#include "renderer.hpp"

#include "entity_builder.hpp"

class Scene : public NoCopyNoMove {
public:
    explicit Scene(Renderer::Window& window);
    ~Scene();

    void add_entity(EntityBuilder& entity);

    // Call update after adding an entity (or maybe I do that internally)
    void update();
    void optimize();

    void physics();
    void draw();

    void set_pass(bool forward);

    void draw_debug_imgui();

    Renderer::Camera& get_camera();
    const Utils::DeltaTime& get_clock();

private:
    void compile_shaders();

    void init_pass();

    Utils::DeltaTime m_clock;

    Renderer::Window& m_window;
    Renderer::Camera m_camera;

    bool m_shaders_need_update = true;
    bool m_forward_pass = true;

    struct DeferedPass {
        Renderer::ShaderProgram m_gpass_shader;
        Renderer::ShaderProgram m_lpass_shader;

        int m_gpass_width = 0;
        int m_gpass_height = 0;
        Renderer::GBuffer m_gpass;
        Renderer::Quad m_lpass;
    };

    struct ForwardPass {
        Renderer::ShaderProgram m_shader;
    };

    DeferedPass* m_deferred = nullptr;
    ForwardPass* m_forward = nullptr;

    // TODO: do I make these global? they never change.
    Renderer::ShaderProgram m_shadowmap_shader;
    Renderer::ShaderProgram m_shadowmap_cubemap_shader;

    entt::registry m_registry;
    Utils::Cache<const char*, Renderer::Model> m_model_cache;

    bool m_physics_needs_optimize = false;
    std::unique_ptr<Physics::System> m_physics_system = nullptr;
};
