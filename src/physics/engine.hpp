#pragma once

#include "../renderer/mesh.hpp"

#include "../utils/math/ray.hpp"

namespace Physics {

namespace Layers {

    static constexpr JPH::ObjectLayer NON_MOVING = 0;
    static constexpr JPH::ObjectLayer MOVING = 1;
    static constexpr JPH::ObjectLayer RENDER_ONLY = 2;
    static constexpr JPH::ObjectLayer NUM_LAYERS = 3;

};

/// Class that determines if two object layers can collide
class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter {
public:
    [[nodiscard]] bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override
    {
        if (inObject1 == Layers::RENDER_ONLY || inObject2 == Layers::RENDER_ONLY) {
            return false;
        }

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
    static constexpr JPH::BroadPhaseLayer RENDER_ONLY(2);
    static constexpr uint NUM_LAYERS(3);

};

class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface {
public:
    BPLayerInterfaceImpl()
    {
        // Create a mapping table from object to broad phase layer
        mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
        mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
        mObjectToBroadPhase[Layers::RENDER_ONLY] = BroadPhaseLayers::RENDER_ONLY;
    }

    [[nodiscard]] uint GetNumBroadPhaseLayers() const override
    {
        return BroadPhaseLayers::NUM_LAYERS;
    }

    [[nodiscard]] JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
    {
        JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
        return mObjectToBroadPhase.at(inLayer);
    }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
    [[nodiscard]] const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override
    {
        switch (static_cast<JPH::BroadPhaseLayer::Type>(inLayer)) {
            case static_cast<JPH::BroadPhaseLayer::Type>(BroadPhaseLayers::NON_MOVING):
                return "NON_MOVING";
            case static_cast<JPH::BroadPhaseLayer::Type>(BroadPhaseLayers::MOVING):
                return "MOVING";
            case static_cast<JPH::BroadPhaseLayer::Type>(BroadPhaseLayers::RENDER_ONLY):
                return "RENDER_ONLY";
            default:
                JPH_ASSERT(false);
                return "INVALID";
        }
    }
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

private:
    std::array<JPH::BroadPhaseLayer, Layers::NUM_LAYERS> mObjectToBroadPhase {};
};

/// Class that determines if an object layer can collide with a broadphase layer
class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter {
public:
    [[nodiscard]] bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override
    {
        if (inLayer1 == Layers::RENDER_ONLY || inLayer2 == BroadPhaseLayers::RENDER_ONLY) {
            return false;
        }

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

class RenderPickingFilter : public JPH::ObjectLayerFilter {
public:
    [[nodiscard]] virtual bool ShouldCollide(JPH::ObjectLayer inLayer) const override
    {
        return inLayer == Layers::RENDER_ONLY || inLayer == Layers::MOVING || inLayer == Layers::NON_MOVING;
    }
};

namespace Engine {
    void setup_singletons();
    void cleanup_singletons();
}

class DebugRenderer : public JPH::DebugRenderer, public NoCopyNoMove {
public:
    DebugRenderer(GlobalAppData* app_data);
    ~DebugRenderer() override = default;

    void DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) override;
    void DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor, ECastShadow inCastShadow = ECastShadow::Off) override;

    void DrawText3D(JPH::RVec3Arg inPosition, const std::string_view& inString, JPH::ColorArg inColor = JPH::Color::sWhite, float inHeight = 0.5f) override;
    Batch CreateTriangleBatch(const Triangle* inTriangles, int inTriangleCount) override;
    Batch CreateTriangleBatch(const Vertex* inVertices, int inVertexCount, const u32* inIndices, int inIndexCount) override;
    void DrawGeometry(JPH::RMat44Arg inModelMatrix, const JPH::AABox& inWorldSpaceBounds, float inLODScaleSq, JPH::ColorArg inModelColor, const GeometryRef& inGeometry, ECullMode inCullMode = ECullMode::CullBackFace, ECastShadow inCastShadow = ECastShadow::Off, EDrawMode inDrawMode = EDrawMode::Wireframe) override;

private:
    GlobalAppData* m_app_data = nullptr;
};

class System : public NoCopyNoMove {
public:
    System();
    ~System();

    void update(float delta_time);

    static void create_mesh_triangle_list(JPH::TriangleList& triangles, const std::deque<Renderer::Mesh>* meshes);
    static void create_mesh_triangle_list(JPH::TriangleList& triangles, const glm::mat4& transform, const std::deque<Renderer::Mesh>* meshes);

    static void create_mesh_triangle_list_base_index(JPH::TriangleList& triangles, const Renderer::Mesh* mesh);
    static void create_mesh_triangle_list_base_index(JPH::TriangleList& triangles, const glm::mat4& transform, const Renderer::Mesh* mesh);

    static void create_mesh_vec3s_base_index(JPH::Array<JPH::Vec3>& triangles, const Renderer::Mesh* mesh);
    static void create_mesh_vec3s_base_index(JPH::Array<JPH::Vec3>& triangles, const glm::mat4& transform, const Renderer::Mesh* mesh);

    std::optional<JPH::BodyID> ray_cast(Utils::Ray ray, float max_distance);

    void add_body(JPH::BodyID body);
    void remove_body(JPH::BodyID body);
    void remove_delete_body(JPH::BodyID body);

    void optimize();

    JPH::BodyInterface* m_body_interface = nullptr;

private:
    JPH::TempAllocatorImpl m_temp_allocator { 10 * 1024 * 1024 };

    const uint cMaxBodies = 65536;
    const uint cNumBodyMutexes = 0;
    const uint cMaxBodyPairs = 65536;
    const uint cMaxContactConstraints = 10240;

    BPLayerInterfaceImpl m_broad_phase_layer_interface;
    ObjectVsBroadPhaseLayerFilterImpl m_object_vs_broadphase_layer_filter;
    ObjectLayerPairFilterImpl m_object_vs_object_layer_filter;

    JPH::PhysicsSystem m_physics_system;
};

} // namespace Physics
