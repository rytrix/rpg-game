#pragma once

#include <assimp/scene.h>

namespace Renderer {

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