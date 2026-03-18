#include "animation.hpp"

#include "../utils/helpers.hpp"

#include "mesh.hpp"

namespace Renderer {

glm::mat4 NodeAnim::keyframe_to_mat4(float animation_time)
{
    aiVector3D pos;
    if (m_position_size == 1) {
        pos = m_position_keys[0].m_value;
    } else {
        auto pos_keyframe = find_keyframe(m_prev_position_frame, m_position_keys, m_position_size, animation_time);
        pos = lerp_interpolate<KeyFrame<aiVector3D>, aiVector3D>(&m_position_keys[pos_keyframe], &m_position_keys[pos_keyframe + 1], animation_time);
        // pos = m_anim->mPositionKeys[pos_keyframes[0]].mValue;
    }

    aiQuaternion rot;
    if (m_rotation_size == 1) {
        rot = m_rotation_keys[0].m_value;
    } else {
        auto rot_keyframe = find_keyframe(m_prev_rotation_frame, m_rotation_keys, m_rotation_size, animation_time);
        rot = slerp_interpolate(&m_rotation_keys[rot_keyframe], &m_rotation_keys[rot_keyframe + 1], animation_time);
        // rot = m_anim->mRotationKeys[rot_keyframes[0]].mValue;
    }

    aiVector3D scale;
    if (m_scaling_size == 1) {
        scale = m_scaling_keys[0].m_value;
    } else {
        auto scale_keyframe = find_keyframe(m_prev_scaling_frame, m_scaling_keys, m_scaling_size, animation_time);
        scale = lerp_interpolate<KeyFrame<aiVector3D>, aiVector3D>(&m_scaling_keys[scale_keyframe], &m_scaling_keys[scale_keyframe + 1], animation_time);
        // scale = m_anim->mScalingKeys[scale_keyframes[0]].mValue;
    }

    return mat4_to_mat4(aiMatrix4x4(scale, rot, pos));
}

template <typename T>
u32 NodeAnim::find_keyframe(u32& cache, const T* keys, u32 keys_size, float animation_time)
{
    // util_assert(animation_time <= keys[keys_size - 1].m_time, "keyframe time is greater than the max keyframe time");
    if (animation_time > keys[keys_size - 1].m_time) {
        return keys_size - 1;
    }

    for (u32 i = cache; i < keys_size - 1; i++) {
        if (animation_time >= keys[i].m_time && animation_time < keys[i + 1].m_time) {
            cache = i;
            return cache;
        }
    }
    for (u32 i = 0; i < cache; i++) {
        if (animation_time < keys[i + 1].m_time) {
            cache = i;
            return cache;
        }
    }
    cache = 0;
    return cache;
    util_error("Could not find keyframe");
}

template <typename T, typename R>
R NodeAnim::lerp_interpolate(T* p_start, T* p_end, float animation_time)
{
    float delta_time = p_end->m_time - p_start->m_time;
    float factor = (animation_time - p_start->m_time) / delta_time;
    // util_assert(factor >= 0.0 && factor <= 1.0, std::format("Lerp({}) factor not in range 0 to 1", factor));
    if (factor < 0.0F || factor > 1.0F) {
        LOG_WARN(std::format("Lerp({}) factor not in range 0 to 1", factor));
    }
    factor = std::min(1.0F, std::max(0.0F, factor));

    return p_start->m_value + factor * (p_end->m_value - p_start->m_value);
}

aiQuaternion NodeAnim::slerp_interpolate(KeyFrame<aiQuaternion>* p_start, KeyFrame<aiQuaternion>* p_end, float animation_time)
{
    aiQuaternion result;
    float delta_time = p_end->m_time - p_start->m_time;
    float factor = (animation_time - p_start->m_time) / delta_time;
    // util_assert(factor >= 0.0F && factor <= 1.0F, std::format("Slerp({}) factor not in range 0 to 1", factor));
    if (factor < 0.0F || factor > 1.0F) {
        LOG_WARN(std::format("Slerp({}) factor not in range 0 to 1", factor));
    }
    factor = std::min(1.0F, std::max(0.0F, factor));

    aiQuaternion::Interpolate(result, p_start->m_value, p_end->m_value, factor);
    result.Normalize();

    return result;
}

void Animation::init(const aiScene* scene, const aiAnimation* animation, const std::unordered_map<std::string, u32>& bone_indices, const glm::mat4& global_inverse_transform, float total_animation_time, float ticks_per_second)
{
    m_name = animation->mName.C_Str();

    m_total_animation_time = total_animation_time;
    m_ticks_per_second = ticks_per_second;
    if (m_ticks_per_second == 0.0F) {
        m_ticks_per_second = 25.0F;
    }

    m_nodes_size = bone_indices.size();
    m_nodes = (NodeAnim*)m_allocator.allocate(sizeof(NodeAnim) * m_nodes_size);

    m_final_transforms.reserve(MAX_BONES);

    util_assert(m_nodes_size <= MAX_BONES,
        std::format("Exceded max bone limit of {} with {} bones",
            MAX_BONES,
            m_nodes_size));
    m_final_transforms.resize(m_nodes_size);

    m_global_inverse_transform = global_inverse_transform;

    evaluate_scene(scene, scene->mRootNode, bone_indices);
    evaluate_parents(scene->mRootNode, bone_indices, UINT32_MAX);

    for (u32 i = 0; i < animation->mNumChannels; i++) {
        aiNodeAnim* ai_node_anim = animation->mChannels[i];

        u32 index = 0;
        if (bone_indices.contains(ai_node_anim->mNodeName.C_Str())) {
            index = bone_indices.at(ai_node_anim->mNodeName.C_Str());
        } else {
            LOG_WARN(std::format("From animation \"{}\", bone \"{}\" not found",
                m_name,
                ai_node_anim->mNodeName.C_Str()));
            continue;
        }

        NodeAnim* node_anim = &m_nodes[index];
        node_anim->m_has_animation = true;

        node_anim->m_position_size = ai_node_anim->mNumPositionKeys;
        node_anim->m_position_keys = (KeyFrame<aiVector3D>*)m_allocator.allocate(node_anim->m_position_size * sizeof(KeyFrame<aiVector3D>));
        for (u32 j = 0; j < ai_node_anim->mNumPositionKeys; j++) {
            node_anim->m_position_keys[j].m_time = static_cast<float>(ai_node_anim->mPositionKeys[j].mTime);
            node_anim->m_position_keys[j].m_value = ai_node_anim->mPositionKeys[j].mValue;
        }

        node_anim->m_rotation_size = ai_node_anim->mNumRotationKeys;
        node_anim->m_rotation_keys = (KeyFrame<aiQuaternion>*)m_allocator.allocate(node_anim->m_rotation_size * sizeof(KeyFrame<aiQuaternion>));
        for (u32 j = 0; j < ai_node_anim->mNumRotationKeys; j++) {
            node_anim->m_rotation_keys[j].m_time = static_cast<float>(ai_node_anim->mRotationKeys[j].mTime);
            node_anim->m_rotation_keys[j].m_value = ai_node_anim->mRotationKeys[j].mValue;
        }

        node_anim->m_scaling_size = ai_node_anim->mNumScalingKeys;
        node_anim->m_scaling_keys = (KeyFrame<aiVector3D>*)m_allocator.allocate(node_anim->m_scaling_size * sizeof(KeyFrame<aiVector3D>));
        for (u32 j = 0; j < ai_node_anim->mNumScalingKeys; j++) {
            node_anim->m_scaling_keys[j].m_time = static_cast<float>(ai_node_anim->mScalingKeys[j].mTime);
            node_anim->m_scaling_keys[j].m_value = ai_node_anim->mScalingKeys[j].mValue;
        }
    }
}

void Animation::update_transforms(float animation_time)
{
    animation_time = std::fmod(animation_time * m_ticks_per_second, m_total_animation_time);

    for (u32 i = 0; i < m_nodes_size; i++) {
        NodeAnim* node = &m_nodes[i];

        glm::mat4 parent_transform;
        if (node->m_parent_index != UINT32_MAX) {
            parent_transform = m_nodes[node->m_parent_index].m_global_transform;
        } else {
            parent_transform = glm::mat4(1.0);
        }

        glm::mat4 node_transform;
        if (node->m_has_animation) {
            node_transform = node->keyframe_to_mat4(animation_time);
        } else {
            node_transform = node->m_node_transform;
        }

        node->m_global_transform = parent_transform * node_transform;

        m_final_transforms[node->m_index] = m_global_inverse_transform * node->m_global_transform * node->m_offset;
    }
}

void Animation::evaluate_scene(const aiScene* scene, const aiNode* node, const std::unordered_map<std::string, u32>& bone_indices)
{
    for (u32 i = 0; i < node->mNumMeshes; i++) {
        auto* mesh = scene->mMeshes[node->mMeshes[i]];
        if (mesh->HasBones()) {
            for (u32 j = 0; j < mesh->mNumBones; j++) {
                aiBone* bone = mesh->mBones[j];
                u32 index = bone_indices.at(bone->mName.C_Str());
                m_nodes[index].m_index = index;
                m_nodes[index].m_has_animation = false;
                m_nodes[index].m_offset = mat4_to_mat4(bone->mOffsetMatrix);
                m_nodes[index].m_prev_position_frame = 0;
                m_nodes[index].m_prev_rotation_frame = 0;
                m_nodes[index].m_prev_scaling_frame = 0;
            }
        }
    }

    for (u32 i = 0; i < node->mNumChildren; i++) {
        evaluate_scene(scene, node->mChildren[i], bone_indices);
    }
}

void Animation::evaluate_parents(const aiNode* node, const std::unordered_map<std::string, u32>& bone_indices, u32 parent_index)
{
    u32 index = parent_index;
    if (bone_indices.contains(node->mName.C_Str())) {
        index = bone_indices.at(node->mName.C_Str());
        m_nodes[index].m_parent_index = parent_index;
        m_nodes[index].m_node_transform = mat4_to_mat4(node->mTransformation);
    }

    for (u32 i = 0; i < node->mNumChildren; i++) {
        evaluate_parents(node->mChildren[i], bone_indices, index);
    }
}

} // namespace Renderer