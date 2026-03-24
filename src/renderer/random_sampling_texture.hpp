#pragma once

#include "../scene/resource_manager2.hpp"

namespace Renderer {

Handle create_random_sampling_texture(i32 window_size, int filter_size, TextureCache* cache);

} // namespace Renderer
