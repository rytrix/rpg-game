#include "app.hpp"

#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/RegisterTypes.h>

static void TraceImpl(const char* inFMT, ...)
{
    // Format the message
    va_list list;
    va_start(list, inFMT);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), inFMT, list);
    va_end(list);

    // Print to the TTY
    std::print("{}", buffer);
}

namespace Layers {
static constexpr JPH::ObjectLayer NON_MOVING = 0;
static constexpr JPH::ObjectLayer MOVING = 1;
static constexpr JPH::ObjectLayer NUM_LAYERS = 2;
};

/// Class that determines if two object layers can collide
class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter {
public:
    [[nodiscard]] virtual bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override
    {
        switch (inObject1) {
            case Layers::NON_MOVING:
                return inObject2 == Layers::MOVING; // Non moving only collides with moving
            case Layers::MOVING:
                return true; // Moving collides with everything
            default:
                JPH_ASSERT(false);
                return false;
        }
    }
};

namespace BroadPhaseLayers {
static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
static constexpr JPH::BroadPhaseLayer MOVING(1);
static constexpr uint NUM_LAYERS(2);
};

class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface {
public:
    BPLayerInterfaceImpl()
    {
        // Create a mapping table from object to broad phase layer
        mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
        mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
    }

    virtual uint GetNumBroadPhaseLayers() const override
    {
        return BroadPhaseLayers::NUM_LAYERS;
    }

    virtual JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
    {
        JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
        return mObjectToBroadPhase[inLayer];
    }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
    [[nodiscard]] virtual const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override
    {
        switch ((JPH::BroadPhaseLayer::Type)inLayer) {
            case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:
                return "NON_MOVING";
            case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:
                return "MOVING";
            default:
                JPH_ASSERT(false);
                return "INVALID";
        }
    }
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

private:
    JPH::BroadPhaseLayer mObjectToBroadPhase[Layers::NUM_LAYERS];
};

/// Class that determines if an object layer can collide with a broadphase layer
class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter {
public:
    virtual bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override
    {
        switch (inLayer1) {
            case Layers::NON_MOVING:
                return inLayer2 == BroadPhaseLayers::MOVING;
            case Layers::MOVING:
                return true;
            default:
                JPH_ASSERT(false);
                return false;
        }
    }
};

class PhysicsEngine {
public:
    PhysicsEngine();
    ~PhysicsEngine();

    void update(float delta_time);

    JPH::TempAllocatorMalloc temp_allocator;
    JPH::JobSystemThreadPool* job_system;

    const uint cMaxBodies = 1024;
    const uint cNumBodyMutexes = 0;
    const uint cMaxBodyPairs = 1024;
    const uint cMaxContactConstraints = 1024;
    const float cDeltaTime = 1.0f / 60.0f;

    BPLayerInterfaceImpl broad_phase_layer_interface;
    ObjectVsBroadPhaseLayerFilterImpl object_vs_broadphase_layer_filter;
    ObjectLayerPairFilterImpl object_vs_object_layer_filter;

    JPH::PhysicsSystem physics_system;

    JPH::BodyInterface* body_interface = nullptr;

    JPH::Body* floor;
    JPH::BodyID box_id;

private:
};

PhysicsEngine::PhysicsEngine()
{
    JPH::RegisterDefaultAllocator();
    JPH::Factory::sInstance = new JPH::Factory();
    JPH::RegisterTypes();
    job_system = new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1);

    physics_system.Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, broad_phase_layer_interface, object_vs_broadphase_layer_filter, object_vs_object_layer_filter);
    body_interface = &physics_system.GetBodyInterface();

    JPH::BoxShapeSettings floor_shape_settings(JPH::Vec3(100.0f, 1.0f, 100.0f));
    floor_shape_settings.SetEmbedded(); // A ref counted object on the stack (base class RefTarget) should be marked as such to prevent it from being freed when its reference count goes to 0.
    JPH::ShapeSettings::ShapeResult floor_shape_result = floor_shape_settings.Create();
    JPH::ShapeRefC floor_shape = floor_shape_result.Get(); // We don't expect an error here, but you can check floor_shape_result for HasError() / GetError()
    JPH::BodyCreationSettings floor_settings(floor_shape, JPH::RVec3(0.0, -1.0, 0.0), JPH::Quat::sIdentity(), JPH::EMotionType::Static, Layers::NON_MOVING);
    floor = body_interface->CreateBody(floor_settings); // Note that if we run out of bodies this can return nullptr
    body_interface->AddBody(floor->GetID(), JPH::EActivation::DontActivate);

    JPH::BodyCreationSettings box_settings(new JPH::BoxShape(JPH::Vec3(0.5, 0.5, 0.5)), JPH::RVec3(0.0, 20.0, 0.0), JPH::Quat::sIdentity(), JPH::EMotionType::Dynamic, Layers::MOVING);
    box_id = body_interface->CreateAndAddBody(box_settings, JPH::EActivation::Activate);
    body_interface->SetLinearVelocity(box_id, JPH::Vec3(0.0f, -0.01f, 0.0f));

    physics_system.OptimizeBroadPhase();
}

PhysicsEngine::~PhysicsEngine()
{
    // Remove the sphere from the physics system. Note that the sphere itself keeps all of its state and can be re-added at any time.
    body_interface->RemoveBody(box_id);

    // Destroy the sphere. After this the sphere ID is no longer valid.
    body_interface->DestroyBody(box_id);

    // Remove and destroy the floor
    body_interface->RemoveBody(floor->GetID());
    body_interface->DestroyBody(floor->GetID());

    // Unregisters all types with the factory and cleans up the default material
    JPH::UnregisterTypes();

    delete job_system;
    job_system = nullptr;

    // Destroy the factory
    delete JPH::Factory::sInstance;
    JPH::Factory::sInstance = nullptr;
}

void PhysicsEngine::update(float delta_time)
{
    // uint step = 0;
    // while (body_interface->IsActive(box_id)) {
    //     // Next step
    //     ++step;
    //     // JPH::RVec3 position = body_interface->GetCenterOfMassPosition(box_id);
    //     // JPH::Vec3 velocity = body_interface->GetLinearVelocity(box_id);

    // }

    // If you take larger steps than 1 / 60th of a second you need to do multiple collision steps in order to keep the simulation stable. Do 1 collision step per 1 / 60th of a second (round up).
    // TODO
    const int cCollisionSteps = 1;

    // Step the world
    physics_system.Update(delta_time, cCollisionSteps, &temp_allocator, job_system);
}

static PhysicsEngine* physics_engine = nullptr;

App::App()
{
    m_window.init("Test Window", 1000, 800);
    m_window.set_relative_mode(true);
    LOG_TRACE("Created window \"Test Window\"");

    m_camera.init(90.0F, 0.1F, 1000.0F, m_window.get_aspect_ratio(), { -2.0F, 1.5F, 4.0F });
    m_camera.set_speed(5.0F);

    m_gpass.init(m_window.get_width(), m_window.get_height());
    m_lpass.init();

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
    m_gpass_shader.init(shader_info.data(), shader_info.size());

    shader_info = {
        Renderer::ShaderInfo {
            .is_file = true,
            .shader = "res/deferred_shading/l_pass.glsl.vert",
            .type = GL_VERTEX_SHADER,
        },
        Renderer::ShaderInfo {
            .is_file = true,
            .shader = "res/deferred_shading/l_pass.glsl.frag",
            .type = GL_FRAGMENT_SHADER,
        },
    };
    m_lpass_shader.init(shader_info.data(), shader_info.size());

    auto shadowmap_info = Renderer::ShadowMap::get_shader_info();
    m_shadowmap_shader.init(shadowmap_info.data(), shadowmap_info.size());

    auto shadowmap_cubemap_info = Renderer::ShadowMap::get_shader_info_cubemap();
    m_shadowmap_cubemap_shader.init(shadowmap_cubemap_info.data(), shadowmap_cubemap_info.size());

    // m_model("res/models/backpack/backpack.obj");
    // m_model("res/models/Sponza/glTF/Sponza.gltf");
    // u_model = glm::scale(glm::mat4 { 1.0 }, glm::vec3(0.1));
    // m_model.init("res/models/cube_texture_mapping/Cube.obj");
    // u_model = glm::scale(glm::mat4 { 1.0 }, glm::vec3(1.0));

    m_plane.init("res/models/physics_plane/plane.obj");
    u_plane = glm::scale(glm::mat4 { 1.0 }, glm::vec3(1.0));

    m_cube.init("res/models/physics_cube/cube.obj");
    u_cube = glm::scale(glm::mat4 { 1.0 }, glm::vec3(1.0));

    m_directional_light.init(
        true,
        glm::vec3(-0.2F, -1.0F, 0.3F),
        glm::vec3(0.1),
        glm::vec3(0.5),
        glm::vec3(0.5));

    m_point_light.init(true,
        glm::vec3(2.0F, 2.0F, 2.0F),
        glm::vec3(0.05F),
        glm::vec3(0.5F),
        glm::vec3(0.5F),
        1.0F,
        0.022F,
        0.0019F);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    keyboard_callback();

    // glEnable(GL_MULTISAMPLE);

    physics_engine = new PhysicsEngine();
}

App::~App()
{
    delete physics_engine;
    physics_engine = nullptr;
}

void App::keyboard_callback()
{
    m_window.process_input_callback([&](SDL_Event& event) {
        if (event.type == SDL_EVENT_WINDOW_RESIZED) {
            m_camera.update_aspect(m_window.get_aspect_ratio());
            m_gpass.reinit(m_window.get_width(), m_window.get_height());
        }
        if (event.type == SDL_EVENT_MOUSE_MOTION) {
            m_camera.rotate(event.motion.xrel, -event.motion.yrel);
        }
        if (event.type == SDL_EVENT_KEY_DOWN) {
            if (event.key.key == SDLK_ESCAPE) {
                m_window.set_should_close();
            }
            if (event.key.key == SDLK_P) {
                glm::vec3 pos = m_camera.get_pos();
                std::println("Camera_position: {}, {}, {}", pos.x, pos.y, pos.z);
            }
        }
    });
}

void App::keyboard_input()
{
    const bool* keys = SDL_GetKeyboardState(nullptr);
    float delta_time = m_clock.delta_time();
    using Dir = Renderer::Camera::Movement;
    if (keys[SDL_SCANCODE_W]) {
        m_camera.move(Dir::Forward, delta_time);
    }
    if (keys[SDL_SCANCODE_S]) {
        m_camera.move(Dir::Backward, delta_time);
    }
    if (keys[SDL_SCANCODE_A]) {
        m_camera.move(Dir::Left, delta_time);
    }
    if (keys[SDL_SCANCODE_D]) {
        m_camera.move(Dir::Right, delta_time);
    }
    if (keys[SDL_SCANCODE_SPACE]) {
        m_camera.move(Dir::Up, delta_time);
    }
    if (keys[SDL_SCANCODE_LSHIFT]) {
        m_camera.move(Dir::Down, delta_time);
    }
}

void App::frame_counter()
{
    static float time_passed;
    static u32 frames;
    static bool initialized;

    if (!initialized) {
        time_passed = 0.0F;
        frames = 0;
        initialized = true;
    } else {
        time_passed += m_clock.delta_time();
        frames += 1;
        if (time_passed >= 1.0F) {
            time_passed = 0;
            std::println("Frames: {}", frames);
            frames = 0;
        }
    }
}

void App::run()
{
    m_window.loop([&]() {
        m_clock.update();
        frame_counter();
        keyboard_input();
        m_camera.update();

        physics_engine->update(m_clock.delta_time());
        JPH::RVec3 box_pos = physics_engine->body_interface->GetCenterOfMassPosition(physics_engine->box_id);
        u_cube[3] = glm::vec4(glm::vec3(box_pos.GetX(), box_pos.GetY(), box_pos.GetZ()), u_cube[3][3]);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        // Shadowmap pass
        glCullFace(GL_FRONT);
        // m_directional_light.shadowmap_draw(m_shadowmap_shader, u_model, [&]() {
        //     m_model.draw();
        // });
        // m_point_light.shadowmap_draw(m_shadowmap_cubemap_shader, u_model, [&]() {
        //     m_model.draw();
        // });

        m_directional_light.shadowmap_draw(m_shadowmap_shader, u_plane, [&]() {
            m_plane.draw();
        });
        m_point_light.shadowmap_draw(m_shadowmap_cubemap_shader, u_plane, [&]() {
            m_plane.draw();
        });

        m_directional_light.shadowmap_draw(m_shadowmap_shader, u_cube, [&]() {
            m_cube.draw();
        });
        m_point_light.shadowmap_draw(m_shadowmap_cubemap_shader, u_cube, [&]() {
            m_cube.draw();
        });

        // Geometry pass
        glCullFace(GL_BACK);
        m_gpass.bind();
        glViewport(0, 0, m_window.get_width(), m_window.get_height());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_gpass_shader.bind();
        m_gpass_shader.set_mat4("proj", m_camera.get_proj());
        m_gpass_shader.set_mat4("view", m_camera.get_view());

        // m_gpass_shader.set_mat4("model", u_model);
        // m_model.draw(m_gpass_shader);

        m_gpass_shader.set_mat4("model", u_plane);
        m_plane.draw(m_gpass_shader);
        m_gpass_shader.set_mat4("model", u_cube);
        m_cube.draw(m_gpass_shader);

        m_gpass.blit_depth_buffer();
        m_gpass.unbind();

        // Lighting pass
        glClear(GL_COLOR_BUFFER_BIT);
        m_lpass_shader.bind();

        m_gpass.set_uniforms(m_lpass_shader);
        m_lpass_shader.set_vec3("view_position", m_camera.get_pos());

        m_directional_light.set_uniforms(m_lpass_shader, "u_directional_light");
        m_point_light.set_uniforms(m_lpass_shader, "u_point_light");
        m_lpass.draw();
    });
}