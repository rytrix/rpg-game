#include "entity_selector.hpp"

#include "../../app_data.hpp"

#include "../../physics/helpers.hpp"

EntitySelector::EntitySelector(Scene* scene, GlobalAppData* app_data)
    : m_app_data(app_data)
    , m_scene(scene)
{
}

void EntitySelector::init(Scene* scene, GlobalAppData* app_data)
{
    m_app_data = app_data;
    m_scene = scene;
}

void EntitySelector::on_event(Event& event)
{
    if (event.m_type == Event::Type::SDL) {
        if (event.m_sdl_event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            if (event.m_sdl_event.button.button == SDL_BUTTON_LEFT) {
                if (!m_selected_entity.valid() && m_hovered_entity.valid()) {
                    select_entity(m_hovered_entity);
                    event.m_consumed = true;
                }
            }
        }
    }
}

void EntitySelector::update()
{
    if (!m_app_data->m_entity_selector.m_selected_entity.valid() && !m_app_data->m_capture_mouse) {
        auto ray_result = m_scene->m_physics_system->ray_cast(Utils::ray_from_mouse(m_app_data), m_app_data->m_camera.get_far());
        if (ray_result.has_value()) {
            auto body_id = ray_result.value();
            auto view = m_scene->m_registry.view<PhysicsInfo>();
            for (auto [entity, body] : view.each()) {
                if (body.m_id == body_id) {
                    m_app_data->m_entity_selector.m_hovered_entity = Entity(m_scene, entity);
                }
            }
        }
    } else {
        m_app_data->m_entity_selector.m_hovered_entity = Entity(m_scene, entt::null);
    }
}

void EntitySelector::draw()
{
    if (m_hovered_entity.valid()) {
        m_scene->draw_entity_wireframe(m_app_data->m_entity_selector.m_hovered_entity, glm::vec4(1.0, 0.0, 0.0, 1.0));
    }

    if (m_app_data->m_entity_selector.m_selected_entity.valid()) {
        auto* transform = &m_app_data->m_entity_selector.m_selected_entity.get_component<Transform>();
        if (m_app_data->m_entity_selector.m_selected_entity.has_component<PhysicsInfo>()) {
            auto& physics_info = m_app_data->m_entity_selector.m_selected_entity.get_component<PhysicsInfo>();
            if (physics_info.m_motion_type != JPH::EMotionType::Static) {
                glm::vec3 pos = Physics::vec3_to_vec3(m_scene->m_physics_system->m_body_interface->GetPosition(physics_info.m_id));
                transform->set_position(pos);

                glm::quat quat = Physics::quat_to_quat(m_scene->m_physics_system->m_body_interface->GetRotation(physics_info.m_id));
                transform->set_rotation(quat);
            }
        }

        m_app_data->m_gizmo.m_transform = transform;
        m_app_data->m_gizmo.update();

        if (m_app_data->m_entity_selector.m_selected_entity.has_component<PhysicsInfo>()) {
            auto& physics_info = m_app_data->m_entity_selector.m_selected_entity.get_component<PhysicsInfo>();

            if (physics_info.m_motion_type != JPH::EMotionType::Static) {
                JPH::Vec3 pos = Physics::vec3_to_vec3(transform->get_position());
                JPH::Quat quat = Physics::quat_to_quat(transform->get_rotation());

                m_scene->m_physics_system->m_body_interface->SetPositionAndRotation(physics_info.m_id, pos, quat, JPH::EActivation::Activate);
            }
        }
        m_app_data->m_gizmo.draw();
    }
}

void EntitySelector::select_entity(Entity entity)
{
    m_selected_entity = entity;
    m_app_data->m_gizmo.m_state = Gizmo::State::Translation;
}
