#include "engine.hpp"

PhysicsEngine::PhysicsEngine()
{
    JPH::RegisterDefaultAllocator();
    JPH::Factory::sInstance = new JPH::Factory();
    JPH::RegisterTypes();
    job_system = new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1);

    physics_system.Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, broad_phase_layer_interface, object_vs_broadphase_layer_filter, object_vs_object_layer_filter);
    body_interface = &physics_system.GetBodyInterface();

    JPH::BodyCreationSettings floor_settings(new JPH::BoxShape(JPH::Vec3(100.0f, 1.0f, 100.0f)), JPH::RVec3(0.0, -1.0, 0.0), JPH::Quat::sIdentity(), JPH::EMotionType::Static, Layers::NON_MOVING);
    JPH::BodyID floor_id = body_interface->CreateAndAddBody(floor_settings, JPH::EActivation::DontActivate); // Note that if we run out of bodies this can return nullptr
    bodies.push_back(floor_id);

    JPH::BodyCreationSettings box_settings(new JPH::BoxShape(JPH::Vec3(0.5, 0.5, 0.5)), JPH::RVec3(0.0, 20.0, 0.0), JPH::Quat::sIdentity(), JPH::EMotionType::Dynamic, Layers::MOVING);
    JPH::BodyID box_id = body_interface->CreateAndAddBody(box_settings, JPH::EActivation::Activate);
    body_interface->SetLinearVelocity(box_id, JPH::Vec3(0.0f, 0.05f, 0.0f));
    bodies.push_back(box_id);
}

PhysicsEngine::~PhysicsEngine()
{
    body_interface->RemoveBodies(bodies.data(), bodies.size());
    body_interface->DestroyBodies(bodies.data(), bodies.size());

    // Unregisters all types with the factory and cleans up the default material
    JPH::UnregisterTypes();

    delete job_system;
    job_system = nullptr;

    delete JPH::Factory::sInstance;
    JPH::Factory::sInstance = nullptr;
}

void PhysicsEngine::update(float delta_time)
{
    // If you take larger steps than 1 / 60th of a second you need to do multiple collision steps in order to keep the simulation stable. Do 1 collision step per 1 / 60th of a second (round up).
    const int collision_steps = std::max(static_cast<int>(std::ceil(delta_time / (1.0f / 60.0f))), 1);

    physics_system.Update(delta_time, collision_steps, &temp_allocator, job_system);
}

void PhysicsEngine::optimize()
{
    physics_system.OptimizeBroadPhase();
}