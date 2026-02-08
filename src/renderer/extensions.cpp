#include "extensions.hpp"

namespace Renderer::Extensions {

namespace {
    std::unique_ptr<std::set<std::string>> gl_extensions = nullptr;

    const std::unique_ptr<std::set<std::string>>& get_opengl_extensions()
    {
        if (gl_extensions == nullptr) {
            gl_extensions = std::make_unique<std::set<std::string>>();
            GLint no_of_extensions = 0;
            glGetIntegerv(GL_NUM_EXTENSIONS, &no_of_extensions);

            for (int i = 0; i < no_of_extensions; ++i) {
                gl_extensions->insert((const char*)glGetStringi(GL_EXTENSIONS, i));
            }
        }

        return gl_extensions;
    }
}

bool is_extension_supported(const char* extension)
{
    // return false;
    return get_opengl_extensions()->contains(extension);
}

} // namespace Renderer::Extensions