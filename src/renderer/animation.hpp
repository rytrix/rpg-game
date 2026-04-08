#pragma once

#include <assimp/scene.h>
#include <memory_resource>

#include "../utils/string.hpp"

namespace Renderer {

template <typename T>
struct KeyFrame {
    float m_time;
    T m_value;
};

struct FrameCache {
    u32 m_prev_position_frame;
    u32 m_prev_rotation_frame;
    u32 m_prev_scaling_frame;
};

struct NodeAnim {
    u32 m_index;
    u32 m_parent_index;
    bool m_has_animation;

    KeyFrame<aiVector3D>* m_scaling_keys;
    usize m_scaling_size;

    KeyFrame<aiQuaternion>* m_rotation_keys;
    usize m_rotation_size;

    KeyFrame<aiVector3D>* m_position_keys;
    usize m_position_size;

    glm::mat4 m_global_transform;
    glm::mat4 m_node_transform;
    glm::mat4 m_offset;

    glm::mat4 keyframe_to_mat4(float animation_time, FrameCache cache);

private:
    template <typename T>
    u32 find_keyframe(u32& cache, const T* keys, u32 keys_size, float animation_time);

    template <typename T, typename R>
    static R lerp_interpolate(T* p_start, T* p_end, float animation_time);
    static aiQuaternion slerp_interpolate(KeyFrame<aiQuaternion>* p_start, KeyFrame<aiQuaternion>* p_end, float animation_time);
};

struct Animation;

struct PerAnimationData {
    FrameCache* m_cache;
    usize m_cache_size;

    glm::mat4* m_final_transforms;
    usize m_final_transforms_size;

    float m_animation_time;
    bool m_paused;

    Animation* m_animation;

    void update_transforms();
};

struct Animation {
    std::string m_name;

    void init(const aiScene* scene, const aiAnimation* animation,
        const std::unordered_map<Utils::String, u32>& bone_indices,
        const glm::mat4& global_inverse_transform,
        float total_animation_time,
        float ticks_per_second);

    PerAnimationData* create_per_animation_data();

    void update_transforms(PerAnimationData* data);

    [[nodiscard]] float get_ticks_per_second() const { return m_ticks_per_second; };
    void set_ticks_per_second(float ticks_per_second) { m_ticks_per_second = ticks_per_second; };

    [[nodiscard]] float get_total_animation_time() const { return m_total_animation_time; };

private:
    // Allocate with an arena
    std::pmr::monotonic_buffer_resource m_allocator;

    // Needs to be sorted in a way such that
    // all parents get calculated before each child
    NodeAnim* m_nodes;
    usize m_nodes_size;

    glm::mat4 m_global_inverse_transform;

    float m_total_animation_time = 0.0;
    float m_ticks_per_second = 0.0;

    void evaluate_scene(const aiScene* scene, const aiNode* node, const std::unordered_map<Utils::String, u32>& bone_indices);
    void evaluate_parents(const aiNode* node, const std::unordered_map<Utils::String, u32>& bone_indices, u32 parent_index);
};

struct AnimationData {
    std::vector<PerAnimationData*> data;
    u32 selected_animation = 0;
};

} // namespace Renderer
