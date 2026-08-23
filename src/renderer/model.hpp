#pragma once

#include "../utils/string.hpp"

class GlobalAppData;

namespace Renderer {
class Mesh;

enum struct ModelResultEnum {
    Ok,
    InvalidFilePath,
    UnknownError,
};

struct ModelResult {
    ModelResultEnum type = ModelResultEnum::Ok;
    Utils::String error;
};

ModelResult load_mesh(Mesh& mesh, const char* path, GlobalAppData* app_data);

} // namespace Renderer

#include "mesh.hpp"