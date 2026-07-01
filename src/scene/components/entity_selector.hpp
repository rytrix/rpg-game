#pragma once

#include "../entity.hpp"
#include "../event.hpp"

class EntitySelector : public NoCopyNoMove {
public:
    EntitySelector() = default;
    EntitySelector(Scene* scene, GlobalAppData* app_data);

    void init(Scene* scene, GlobalAppData* app_data);

    void on_event(Event& event);
    // Do after scene draw command
    void update();
    void draw();

    void select_entity(Entity entity);

    Entity m_hovered_entity;
    Entity m_selected_entity;

private:
    GlobalAppData* m_app_data = nullptr;
    Scene* m_scene = nullptr;
};
