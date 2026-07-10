#pragma once

class GlobalAppData;

#include "mesh.hpp"

namespace Renderer {

enum struct ModelResultEnum {
    Ok,
    InvalidFilePath,
    UnknownError,
};

struct ModelResult {
    ModelResultEnum type;
    Utils::String error;
};

ModelResult load_mesh(Mesh& mesh, const char* path, GlobalAppData* app_data);

} // namespace Renderer
