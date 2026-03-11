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
    if (m_anim == nullptr) {
        return { 1.0F };
    }
    util_assert(initialized == true, "BoneAnimation has not been initialized");
    aiVector3D pos;
    if (m_anim->mNumPositionKeys == 1) {
        pos = m_anim->mPositionKeys[0].mValue;
    } else {
        auto pos_keyframes = find_keyframe(m_prev_pos_frame, m_anim->mPositionKeys, m_anim->mNumPositionKeys, animation_time);
        pos = lerp_interpolate<aiVectorKey, aiVector3D>(&m_anim->mPositionKeys[pos_keyframes[0]], &m_anim->mPositionKeys[pos_keyframes[1]], animation_time);
    }
    aiMatrix4x4 translation_matrix;
    aiMatrix4x4::Translation(pos, translation_matrix);

    aiQuaternion rot;
    if (m_anim->mNumRotationKeys == 1) {
        rot = m_anim->mRotationKeys[0].mValue;
    } else {
        auto rot_keyframes = find_keyframe(m_prev_rot_frame, m_anim->mRotationKeys, m_anim->mNumRotationKeys, animation_time);
        rot = slerp_interpolate(&m_anim->mRotationKeys[rot_keyframes[0]], &m_anim->mRotationKeys[rot_keyframes[1]], animation_time);
    }
    aiMatrix4x4 rotation_matrix = aiMatrix4x4(rot.GetMatrix());

    aiVector3D scale;
    if (m_anim->mNumScalingKeys == 1) {
        scale = m_anim->mScalingKeys[0].mValue;
    } else {
        auto scale_keyframes = find_keyframe(m_prev_scale_frame, m_anim->mScalingKeys, m_anim->mNumScalingKeys, animation_time);
        scale = lerp_interpolate<aiVectorKey, aiVector3D>(&m_anim->mScalingKeys[scale_keyframes[0]], &m_anim->mScalingKeys[scale_keyframes[1]], animation_time);
    }
    aiMatrix4x4 scale_matrix;
    aiMatrix4x4::Scaling(scale, scale_matrix);

    // glm::mat4 test_matrix = glm::rotate(glm::mat4(1.0F), glm::radians(animation_time), glm::vec3(0.5));
    // return test_matrix;
    return mat4_to_mat4(translation_matrix * rotation_matrix * scale_matrix);
}

template <typename T>
std::array<u32, 2> BoneAnimation::find_keyframe(u32& cache, const T* keys, u32 keys_size, float animation_time)
{
    util_assert(initialized == true, "BoneAnimation has not been initialized");
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
    float delta_time = static_cast<float>(p_end->mTime - p_start->mTime);
    float factor = (animation_time - static_cast<float>(p_start->mTime)) / delta_time;
    util_assert(factor >= 0.0F && factor <= 1.0F, std::format("Lerp({}) factor not in range 0 to 1", factor));

    return p_start->mValue + factor * (p_end->mValue - p_start->mValue);
}

aiQuaternion BoneAnimation::slerp_interpolate(aiQuatKey* p_start, aiQuatKey* p_end, float animation_time)
{
    util_assert(initialized == true, "BoneAnimation has not been initialized");
    aiQuaternion result;
    float delta_time = static_cast<float>(p_end->mTime - p_start->mTime);
    float factor = (animation_time - static_cast<float>(p_start->mTime)) / delta_time;
    util_assert(factor >= 0.0F && factor <= 1.0F, std::format("Slerp({}) factor not in range 0 to 1", factor));

    aiQuaternion::Interpolate(result, p_start->mValue, p_end->mValue, factor);
    result.Normalize();

    return result;
}

} // namespace Renderer