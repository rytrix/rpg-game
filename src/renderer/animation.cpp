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

glm::mat4 BoneAnimation::keyframe_to_mat4(double animation_time)
{
    util_assert(initialized == true, "BoneAnimation has not been initialized");

    aiVector3D pos;
    if (m_anim->mNumPositionKeys == 1) {
        pos = m_anim->mPositionKeys[0].mValue;
    } else {
        auto pos_keyframe = find_keyframe(m_prev_pos_frame, m_anim->mPositionKeys, m_anim->mNumPositionKeys, animation_time);
        pos = lerp_interpolate<aiVectorKey, aiVector3D>(&m_anim->mPositionKeys[pos_keyframe], &m_anim->mPositionKeys[pos_keyframe + 1], animation_time);
        // pos = m_anim->mPositionKeys[pos_keyframes[0]].mValue;
    }

    aiQuaternion rot;
    if (m_anim->mNumRotationKeys == 1) {
        rot = m_anim->mRotationKeys[0].mValue;
    } else {
        auto rot_keyframe = find_keyframe(m_prev_rot_frame, m_anim->mRotationKeys, m_anim->mNumRotationKeys, animation_time);
        rot = slerp_interpolate(&m_anim->mRotationKeys[rot_keyframe], &m_anim->mRotationKeys[rot_keyframe + 1], animation_time);
        // rot = m_anim->mRotationKeys[rot_keyframes[0]].mValue;
    }

    aiVector3D scale;
    if (m_anim->mNumScalingKeys == 1) {
        scale = m_anim->mScalingKeys[0].mValue;
    } else {
        auto scale_keyframe = find_keyframe(m_prev_scale_frame, m_anim->mScalingKeys, m_anim->mNumScalingKeys, animation_time);
        scale = lerp_interpolate<aiVectorKey, aiVector3D>(&m_anim->mScalingKeys[scale_keyframe], &m_anim->mScalingKeys[scale_keyframe + 1], animation_time);
        // scale = m_anim->mScalingKeys[scale_keyframes[0]].mValue;
    }

    return mat4_to_mat4(aiMatrix4x4(scale, rot, pos));
}

template <typename T>
u32 BoneAnimation::find_keyframe(u32& cache, const T* keys, u32 keys_size, double animation_time)
{
    util_assert(initialized == true, "BoneAnimation has not been initialized");
    // util_assert(animation_time <= keys[keys_size - 1].mTime, "keyframe time is greater than the max keyframe time");
    if (animation_time > keys[keys_size - 1].mTime) {
        return keys_size - 1;
    }

    for (u32 i = cache; i < keys_size - 1; i++) {
        if (animation_time >= keys[i].mTime && animation_time < keys[i + 1].mTime) {
            cache = i;
            return cache;
        }
    }
    for (u32 i = 0; i < cache; i++) {
        if (animation_time < keys[i + 1].mTime) {
            cache = i;
            return cache;
        }
    }
    util_error("Could not find keyframe");
}

template <typename T, typename R>
R BoneAnimation::lerp_interpolate(T* p_start, T* p_end, double animation_time)
{
    util_assert(initialized == true, "BoneAnimation has not been initialized");
    double delta_time = p_end->mTime - p_start->mTime;
    double factor = (animation_time - p_start->mTime) / delta_time;
    // util_assert(factor >= 0.0 && factor <= 1.0, std::format("Lerp({}) factor not in range 0 to 1", factor));
    if (factor < 0.0 || factor > 1.0) {
        LOG_WARN(std::format("Lerp({}) factor not in range 0 to 1", factor));
    }
    factor = std::min(1.0, std::max(0.0, factor));

    return p_start->mValue + static_cast<float>(factor) * (p_end->mValue - p_start->mValue);
}

aiQuaternion BoneAnimation::slerp_interpolate(aiQuatKey* p_start, aiQuatKey* p_end, double animation_time)
{
    util_assert(initialized == true, "BoneAnimation has not been initialized");
    aiQuaternion result;
    double delta_time = p_end->mTime - p_start->mTime;
    double factor = (animation_time - p_start->mTime) / delta_time;
    // util_assert(factor >= 0.0 && factor <= 1.0, std::format("Slerp({}) factor not in range 0 to 1", factor));
    if (factor < 0.0 || factor > 1.0) {
        LOG_WARN(std::format("Slerp({}) factor not in range 0 to 1", factor));
    }
    factor = std::min(1.0, std::max(0.0, factor));

    aiQuaternion::Interpolate(result, p_start->mValue, p_end->mValue, static_cast<float>(factor));
    result.Normalize();

    return result;
}

} // namespace Renderer