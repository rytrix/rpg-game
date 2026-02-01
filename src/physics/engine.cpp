#include "engine.hpp"

#include "helpers.hpp"

namespace Physics {

namespace {
    JPH::JobSystemThreadPool* s_job_system = nullptr;
}

namespace Engine {

    void setup_singletons()
    {
        JPH::RegisterDefaultAllocator();
        JPH::Factory::sInstance = new JPH::Factory();
        JPH::RegisterTypes();
        s_job_system = new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1);
    }

    void cleanup_singletons()
    {
        delete s_job_system;
        s_job_system = nullptr;

        delete JPH::Factory::sInstance;
        JPH::Factory::sInstance = nullptr;
    }

} // namespace Engine

System::System()
{
    m_physics_system.Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, m_broad_phase_layer_interface, m_object_vs_broadphase_layer_filter, m_object_vs_object_layer_filter);
    m_body_interface = &m_physics_system.GetBodyInterface();
}

System::~System()
{
    m_body_interface->RemoveBodies(m_bodies.data(), m_bodies.size());
    m_body_interface->DestroyBodies(m_bodies.data(), m_bodies.size());

    // Unregisters all types with the factory and cleans up the default material
    JPH::UnregisterTypes();
}

void System::update(float delta_time)
{
    // If you take larger steps than 1 / 60th of a second you
    // need to do multiple collision steps in order to keep the simulation stable.
    // Do 1 collision step per 1 / 60th of a second (round up).
    // For some reason I have to cap it at a number or it will segfault the program
    const int collision_steps = std::min(std::max(static_cast<int>(std::ceil(delta_time / (1.0f / 60.0f))), 1), 10);

    m_physics_system.Update(delta_time, collision_steps, &m_temp_allocator, s_job_system);
}

void System::create_mesh_triangle_list(JPH::TriangleList& triangles, const std::deque<Renderer::Mesh>* meshes)
{
    for (std::size_t i = 0; i < meshes->size(); i++) {
        const Renderer::Mesh* mesh = &meshes->at(i);
        glm::vec3 v1;
        glm::vec3 v2;
        glm::vec3 v3;
        u32 j = 0;
        while (j + 2 < mesh->m_indices.size()) {
            v1 = mesh->m_vertices[mesh->m_indices.at(j + 0)].m_pos;
            v2 = mesh->m_vertices[mesh->m_indices.at(j + 1)].m_pos;
            v3 = mesh->m_vertices[mesh->m_indices.at(j + 2)].m_pos;
            j += 3;

            JPH::Triangle triangle(vec3_to_float3(v1), vec3_to_float3(v2), vec3_to_float3(v3));
            triangles.push_back(triangle);
        }
    }
}

void System::create_mesh_triangle_list(JPH::TriangleList& triangles, const glm::mat4& model, const std::deque<Renderer::Mesh>* meshes)
{
    for (std::size_t i = 0; i < meshes->size(); i++) {
        const Renderer::Mesh* mesh = &meshes->at(i);
        glm::vec3 v1;
        glm::vec3 v2;
        glm::vec3 v3;
        u32 j = 0;
        while (j + 2 < mesh->m_indices.size()) {
            v1 = glm::vec4(mesh->m_vertices[mesh->m_indices.at(j + 0)].m_pos, 1.0F) * model;
            v2 = glm::vec4(mesh->m_vertices[mesh->m_indices.at(j + 1)].m_pos, 1.0F) * model;
            v3 = glm::vec4(mesh->m_vertices[mesh->m_indices.at(j + 2)].m_pos, 1.0F) * model;
            j += 3;

            JPH::Triangle triangle(vec3_to_float3(v1), vec3_to_float3(v2), vec3_to_float3(v3));
            triangles.push_back(triangle);
        }
    }
}

void System::create_mesh_triangle_list_base_index(JPH::TriangleList& triangles, const Renderer::Mesh* mesh)
{
    usize offset = 0;
    for (std::size_t i = 0; i < mesh->m_base_vertices.size(); i++) {
        glm::vec3 v1;
        glm::vec3 v2;
        glm::vec3 v3;

        auto base = mesh->m_base_vertices[i].m_base;
        auto count = mesh->m_base_vertices[i].m_count;
        u32 j = offset;
        while (j + 2 < count + offset) {
            v1 = mesh->m_vertices[mesh->m_indices.at(j + 0) + base].m_pos;
            v2 = mesh->m_vertices[mesh->m_indices.at(j + 1) + base].m_pos;
            v3 = mesh->m_vertices[mesh->m_indices.at(j + 2) + base].m_pos;
            j += 3;

            JPH::Triangle triangle(vec3_to_float3(v1), vec3_to_float3(v2), vec3_to_float3(v3));
            triangles.push_back(triangle);
        }

        offset += count;
    }
}

void System::create_mesh_triangle_list_base_index(JPH::TriangleList& triangles, const glm::mat4& model, const Renderer::Mesh* mesh)
{
    usize offset = 0;
    for (std::size_t i = 0; i < mesh->m_base_vertices.size(); i++) {
        glm::vec3 v1;
        glm::vec3 v2;
        glm::vec3 v3;

        auto base = mesh->m_base_vertices[i].m_base;
        auto count = mesh->m_base_vertices[i].m_count;
        u32 j = offset;
        while (j + 2 < count + offset) {
            v1 = glm::vec4(mesh->m_vertices[mesh->m_indices.at(j + 0) + base].m_pos, 1.0F) * model;
            v2 = glm::vec4(mesh->m_vertices[mesh->m_indices.at(j + 1) + base].m_pos, 1.0F) * model;
            v3 = glm::vec4(mesh->m_vertices[mesh->m_indices.at(j + 2) + base].m_pos, 1.0F) * model;
            j += 3;

            JPH::Triangle triangle(vec3_to_float3(v1), vec3_to_float3(v2), vec3_to_float3(v3));
            triangles.push_back(triangle);
        }

        offset += count;
    }
}

void System::optimize()
{
    m_physics_system.OptimizeBroadPhase();
}

} // namespace Physics