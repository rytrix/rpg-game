#include "engine.hpp"

#include "helpers.hpp"

PhysicsEngine::PhysicsEngine()
{
    JPH::RegisterDefaultAllocator();
    JPH::Factory::sInstance = new JPH::Factory();
    JPH::RegisterTypes();
    m_job_system = new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1);

    m_physics_system.Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, m_broad_phase_layer_interface, m_object_vs_broadphase_layer_filter, m_object_vs_object_layer_filter);
    m_body_interface = &m_physics_system.GetBodyInterface();
}

PhysicsEngine::~PhysicsEngine()
{
    m_body_interface->RemoveBodies(m_bodies.data(), m_bodies.size());
    m_body_interface->DestroyBodies(m_bodies.data(), m_bodies.size());

    // Unregisters all types with the factory and cleans up the default material
    JPH::UnregisterTypes();

    delete m_job_system;
    m_job_system = nullptr;

    delete JPH::Factory::sInstance;
    JPH::Factory::sInstance = nullptr;
}

void PhysicsEngine::update(float delta_time)
{
    // If you take larger steps than 1 / 60th of a second you need to do multiple collision steps in order to keep the simulation stable. Do 1 collision step per 1 / 60th of a second (round up).
    const int collision_steps = std::max(static_cast<int>(std::ceil(delta_time / (1.0f / 60.0f))), 1);

    m_physics_system.Update(delta_time, collision_steps, &m_temp_allocator, m_job_system);
}

void PhysicsEngine::create_mesh_triangle_list(JPH::TriangleList& triangles, const std::deque<Renderer::Mesh>* meshes)
{
    for (std::size_t i = 0; i < meshes->size(); i++) {
        const Renderer::Mesh* mesh = &meshes->at(i);
        glm::vec3 v1;
        glm::vec3 v2;
        glm::vec3 v3;
        u32 j = 0;
        while (j + 2 < mesh->m_indicies.size()) {
            v1 = mesh->m_verticies[mesh->m_indicies.at(j + 0)].m_pos;
            v2 = mesh->m_verticies[mesh->m_indicies.at(j + 1)].m_pos;
            v3 = mesh->m_verticies[mesh->m_indicies.at(j + 2)].m_pos;
            j += 3;

            JPH::Triangle triangle(vec3_to_float3(v1), vec3_to_float3(v2), vec3_to_float3(v3));
            triangles.push_back(triangle);
        }
    }
}

void PhysicsEngine::optimize()
{
    m_physics_system.OptimizeBroadPhase();
}