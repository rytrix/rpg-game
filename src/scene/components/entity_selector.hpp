#pragma once

#include "../entity.hpp"
#include "../event.hpp"

class EntitySelector : public NoCopyNoMove {
public:
    EntitySelector() = default;
    EntitySelector(Scene* scene, GlobalAppData* app_data);

    void init(Scene* scene, GlobalAppData* app_data);

    void on_event(Event& event);
    void update();
    void draw();

    void select_entity(Entity entity);
    void deselect_entity();

    Entity m_hovered_entity;
    Entity m_selected_entity;

private:
    GlobalAppData* m_app_data = nullptr;
    Scene* m_scene = nullptr;

    enum State {
        On,
        Off,
        Invalid
    };
    State m_prev_physics_state = State::Off;

    struct EntityComponents {
        Entity entity;
        Scene* scene = nullptr;
        Utils::String* name = nullptr;
        Renderer::Mesh** mesh = nullptr;
        Renderer::AnimationData* animation_data = nullptr;
        Transform* transform = nullptr;
        Physics::PhysicsInfo* physics_info = nullptr;
        Renderer::Light::Pbr::Point* point = nullptr;
        Renderer::Light::Pbr::PointShadow* point_shadow = nullptr;
        Renderer::Light::Pbr::Directional* directional = nullptr;
        Renderer::Light::Pbr::DirectionalShadow* directional_shadow = nullptr;
        Renderer::Light::Pbr::Spot* spot = nullptr;
        Renderer::Light::Pbr::SpotShadow* spot_shadow = nullptr;
    };

    struct AddModelPrompt {
        bool valid = false;
        Entity entity;
        Utils::String path;
    };
    AddModelPrompt m_model_prompt {};

    bool m_imgui_first_time = true;
    void draw_selected_entity_imgui();
    void draw_add_remove_component_imgui(EntityComponents& components);
    void draw_model_prompt_window();
};
