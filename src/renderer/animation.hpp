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
    bool m_has_animation;

    KeyFrame<aiVector3D>* m_scaling_keys;
    usize m_scaling_size;

    KeyFrame<aiQuaternion>* m_rotation_keys;
    usize m_rotation_size;

    KeyFrame<aiVector3D>* m_position_keys;
    usize m_position_size;

    u32 m_prev_position_frame;
    u32 m_prev_rotation_frame;
    u32 m_prev_scaling_frame;

    glm::mat4 m_global_transform;
    glm::mat4 m_node_transform;
    glm::mat4 m_offset;

    glm::mat4 keyframe_to_mat4(double animation_time);

private:
    template <typename T>
    u32 find_keyframe(u32& cache, const T* keys, u32 keys_size, double animation_time);

    template <typename T, typename R>
    R lerp_interpolate(T* p_start, T* p_end, double animation_time);
    aiQuaternion slerp_interpolate(KeyFrame<aiQuaternion>* p_start, KeyFrame<aiQuaternion>* p_end, double animation_time);
};

struct Animation {
    std::string m_name;

    // Needs to be sorted in a way such that
    // all parents get calculated before each child
    NodeAnim* m_nodes;
    usize m_nodes_size;

    std::vector<glm::mat4> m_final_transforms;
    glm::mat4 m_global_inverse_transform;

    void init(const aiScene* scene, const aiAnimation* animation,
        const std::unordered_map<std::string, u32>& bone_indices,
        const glm::mat4& global_inverse_transform,
        double total_animation_time,
        double ticks_per_second);

    void update_transforms(double animation_time);

    [[nodiscard]] double get_ticks_per_second() const { return m_ticks_per_second; };
    void set_ticks_per_second(double ticks_per_second) { m_ticks_per_second = ticks_per_second; };

    [[nodiscard]] double get_total_animation_time() const { return m_total_animation_time; };

private:
    // Allocate with an arena
    std::pmr::monotonic_buffer_resource m_allocator;

    double m_total_animation_time = 0.0;
    double m_ticks_per_second = 0.0;

    void evaluate_scene(const aiScene* scene, const aiNode* node, const std::unordered_map<std::string, u32>& bone_indices);
    void evaluate_parents(const aiNode* node, const std::unordered_map<std::string, u32>& bone_indices, u32 parent_index);
};

} // namespace Renderer