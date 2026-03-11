#include "animation.hpp"

#include "../utils/helpers.hpp"

namespace Renderer {

BoneAnimation::BoneAnimation(aiNodeAnim* anim)
{
    init(anim);
}

void BoneAnimation::init(aiNodeAnim* anim)
{
    if (anim == nullptr) {
        return;
    }
    m_anim = anim;
    m_prev_pos_frame = 0;
    m_prev_rot_frame = 0;
    m_prev_scale_frame = 0;

    initialized = true;
}

BoneAnimation::~BoneAnimation()
{
    if (initialized) {
        m_anim = nullptr;
        initialized = false;
    }
}

glm::mat4 BoneAnimation::keyframe_to_mat4(float animation_time)
{
    util_assert(initialized == true, "BoneAnimation has not been initialized");

    glm::mat4 translation_matrix {1.0F};
    glm::mat4 rotation_matrix {1.0F};
    glm::mat4 scale_matrix {1.0F};

    auto pos_keyframes = find_keyframe(m_prev_pos_frame, m_anim->mPositionKeys, m_anim->mNumPositionKeys, animation_time);
    aiVector3D pos = lerp_interpolate<aiVectorKey, aiVector3D>(&m_anim->mPositionKeys[pos_keyframes[0]], &m_anim->mPositionKeys[pos_keyframes[1]], animation_time);
    glm::vec3 glm_pos = vec3_to_vec3(pos);
    translation_matrix = glm::translate(glm::mat4(1.0F), glm_pos);

    auto rot_keyframes = find_keyframe(m_prev_rot_frame, m_anim->mRotationKeys, m_anim->mNumRotationKeys, animation_time);
    aiQuaternion rot = slerp_interpolate(&m_anim->mRotationKeys[rot_keyframes[0]], &m_anim->mRotationKeys[rot_keyframes[1]], animation_time);
    glm::quat glm_rot = glm::quat(rot.w, rot.x, rot.y, rot.z);
    rotation_matrix = glm::mat4(glm_rot);

    auto scale_keyframes = find_keyframe(m_prev_scale_frame, m_anim->mScalingKeys, m_anim->mNumScalingKeys, animation_time);
    aiVector3D scale = lerp_interpolate<aiVectorKey, aiVector3D>(&m_anim->mScalingKeys[scale_keyframes[0]], &m_anim->mScalingKeys[scale_keyframes[1]], animation_time);
    glm::vec3 glm_scale = vec3_to_vec3(scale);
    scale_matrix = glm::scale(glm::mat4(1.0F), glm_scale);

    // glm::mat4 test_matrix = glm::rotate(glm::mat4(1.0F), glm::radians(animation_time), glm::vec3(0.5));

    // return test_matrix;
    return translation_matrix * rotation_matrix * scale_matrix;
}

template <typename T>
std::array<u32, 2> BoneAnimation::find_keyframe(u32& cache, const T* keys, u32 keys_size, float animation_time)
{
    util_assert(initialized == true, "BoneAnimation has not been initialized");
    // If time is greater than the last keyframe just throw an error
    util_assert(animation_time <= keys[keys_size - 1].mTime, "keyframe time is greater than the max keyframe time");

    // Cache check
    if (animation_time >= keys[cache].mTime && animation_time < keys[cache + 1].mTime) {
        return { cache, cache + 1 };
    }

    // Binary search
    // TODO binary search from cache to high, then low to cache
    u32 low = 0;
    u32 high = keys_size - 1;

    while (low < high) {
        u32 mid = low + (high - low) / 2;
        if (animation_time >= static_cast<float>(keys[mid].mTime)) {
            low = mid + 1;
        } else {
            high = mid;
        }
    }

    // The result low is the first key after time
    if (low > 0) {
        cache = low - 1;
    } else {
        cache = 0;
    }

    return { cache, cache + 1 };
}

template <typename T, typename R>
R BoneAnimation::lerp_interpolate(T* p_start, T* p_end, float animation_time)
{
    util_assert(initialized == true, "BoneAnimation has not been initialized");
    float delta_time = static_cast<float>(p_start->mTime - p_end->mTime);
    float factor = (animation_time - static_cast<float>(p_start->mTime)) / delta_time;

    return p_start->mValue + factor * (p_end->mValue - p_start->mValue);
}

aiQuaternion BoneAnimation::slerp_interpolate(aiQuatKey* p_start, aiQuatKey* p_end, float animation_time)
{
    util_assert(initialized == true, "BoneAnimation has not been initialized");
    aiQuaternion result;
    // float delta_time = static_cast<float>(p_start->mTime - p_end->mTime);
    // float factor = (animation_time - static_cast<float>(p_start->mTime)) / delta_time;

    float start_time = (float)p_start->mTime;
    float end_time = (float)p_end->mTime;
    float delta_time = end_time - start_time;

    if (delta_time <= 0.0f) return p_start->mValue;

    float factor = (animation_time - start_time) / delta_time;
    factor = glm::clamp(factor, 0.0f, 1.0f); // Crucial!

    aiQuaternion::Interpolate(result, p_start->mValue, p_end->mValue, factor);

    return result;
}

} // namespace Renderer