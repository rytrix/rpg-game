#include "window.hpp"
#include "SDL3/SDL_video.h"

namespace Renderer {

namespace {

#ifdef _DEBUG
    void GLAPIENTRY
    MessageCallback([[maybe_unused]] GLenum source,
        GLenum type,
        [[maybe_unused]] GLuint id,
        GLenum severity,
        [[maybe_unused]] GLsizei length,
        const GLchar* message,
        [[maybe_unused]] const void* userParam)
    {
        std::println(stderr, "GL CALLBACK: {} type = 0x{:x}, severity = 0x{:x}, message = {}",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
            type, severity, message);
    }
#endif

    Window* currentWindowPtr = nullptr;

} // anonymous namespace

Window::Window(const char* name, int width, int height)
    : m_width(width)
    , m_height(height)
{

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        throw std::runtime_error(std::format("Could not initialize SDL: {}", SDL_GetError()));
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    // SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    // SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    m_window = SDL_CreateWindow(name,
        width, height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if (m_window == nullptr) {
        SDL_Quit();
        throw std::runtime_error(std::format("Could not create window: {}", SDL_GetError()));
    }

    m_context = SDL_GL_CreateContext(m_window);
    if (m_context == nullptr) {
        SDL_DestroyWindow(m_window);
        SDL_Quit();
        throw std::runtime_error(std::format("Could not create OpenGL context: {}", SDL_GetError()));
    }

    if (gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress) == 0) {
        SDL_GL_DestroyContext(m_context);
        SDL_DestroyWindow(m_window);
        SDL_Quit();
        throw std::runtime_error("Failed to initialize GLAD");
    }

    SDL_GL_MakeCurrent(m_window, m_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

#ifdef _DEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, 0);
#endif

    glViewport(0, 0, width, height);
    // glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    //
    // glfwSetCursorPosCallback(window, windowMouseCallback);
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    currentWindowPtr = this;
}

Window::~Window()
{
    SDL_GL_DestroyContext(m_context);
    SDL_DestroyWindow(m_window);
    SDL_Quit();
}

void Window::process_input_callback(const std::function<void(SDL_Event& event)>& commands)
{
    m_process_input_fn = commands;
}

void Window::process_input_internal()
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            m_should_close = true;
        }
        if (event.type == SDL_EVENT_WINDOW_RESIZED) {
            SDL_GetWindowSize(m_window, &m_width, &m_height);
            glViewport(0, 0, m_width, m_height);
        }

        m_process_input_fn(event);
    }
}

void Window::loop(const std::function<void()>& commands)
{
    while (!m_should_close) {
        process_input_internal();
        if (m_should_close) {
            break;
        }

        commands();

        SDL_GL_SwapWindow(m_window);
    }
}

// void Window::windowMouseCallback(UNUSED GLFWwindow* _window, double x, double y)
// {
//     if (currentWindowPtr != nullptr && currentWindowPtr->mouseCallbackFn != nullptr) {
//         currentWindowPtr->mouseCallbackFn(x, y);
//     }
// }

void Window::set_capture_mouse(bool value)
{
    SDL_CaptureMouse(value);
}

void Window::set_relative_mode(bool value)
{
    SDL_SetWindowRelativeMouseMode(m_window, value);
}

[[nodiscard]] SDL_Window* Window::get_window_ptr() const
{
    return m_window;
}

[[nodiscard]] std::pair<int, int> Window::get_size() const
{
    return { m_width, m_height };
}

[[nodiscard]] std::pair<f32, f32> Window::get_size_f32() const
{
    return { static_cast<f32>(m_width), static_cast<f32>(m_height) };
}

[[nodiscard]] f32 Window::get_aspect_ratio() const
{
    auto size = get_size_f32();
    return size.first / size.second;
}

void Window::set_should_close()
{
    m_should_close = true;
}

} // namespace Renderer
