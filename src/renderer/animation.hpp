#pragma once

#include <assimp/scene.h>
#include <memory_resource>

namespace Renderer {

template <typename T>
struct KeyFrame {
    double m_time;
    T m_value;
};

struct NodeAnim {
    u32 m_index;
    u32 m_parent_index;

    KeyFrame<glm::vec3>* m_scaling;
    usize m_scaling_size;

    KeyFrame<glm::quat>* m_rotation;
    usize m_rotation_size;

    KeyFrame<glm::vec3>* m_translation;
    usize m_translation_size;

    glm::mat4 m_offset;
};

struct Animation {
    std::string m_name;

    // Needs to be sorted in a way such that 
    // all parents get calculated before each child
    NodeAnim* m_nodes;
    usize m_node_size;

    // Allocate with an arena
    std::pmr::monotonic_buffer_resource m_allocator;
};

class BoneAnimation {
public:
    explicit BoneAnimation(aiNodeAnim* anim);
    BoneAnimation() = default;

    ~BoneAnimation();
    void init(aiNodeAnim* anim);

    glm::mat4 keyframe_to_mat4(double animation_time);
    [[nodiscard]] bool is_initialized() const { return initialized; }

private:
    bool initialized = false;
    aiNodeAnim* m_anim = nullptr;

    u32 m_prev_pos_frame = 0;
    u32 m_prev_rot_frame = 0;
    u32 m_prev_scale_frame = 0;

    template <typename T>
    u32 find_keyframe(u32& cache, const T* keys, u32 keys_size, double animation_time);

    template <typename T, typename R>
    R lerp_interpolate(T* p_start, T* p_end, double animation_time);
    aiQuaternion slerp_interpolate(aiQuatKey* p_start, aiQuatKey* p_end, double animation_time);
};

} //  namespace Renderer