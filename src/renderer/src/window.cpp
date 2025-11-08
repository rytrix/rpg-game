#include "window.hpp"

namespace Renderer {

// namespace {

// #ifdef _DEBUG
//     void GLAPIENTRY
//     MessageCallback(UNUSED GLenum source,
//         GLenum type,
//         UNUSED GLuint id,
//         GLenum severity,
//         UNUSED GLsizei length,
//         const GLchar* message,
//         UNUSED const void* userParam)
//     {
//         fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
//             (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
//             type, severity, message);
//     }
// #endif

//     static Window* currentWindowPtr = nullptr;

// } // anonymous namespace

// Window::Window(const char* name, int width, int height)
//     : width(width)
//     , height(height)
// {
//     glfwInit();
//     glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
//     glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
//     glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

//     window = glfwCreateWindow(width, height, name, NULL, NULL);
//     if (window == nullptr) {
//         std::print("Failed to create GLFW window\n");
//         glfwTerminate();
//         exit(EXIT_FAILURE);
//     }
//     glfwMakeContextCurrent(window);
//     glfwSwapInterval(1);

//     if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
//         std::print("Failed to initialize GLAD\n");
//         exit(EXIT_FAILURE);
//     }

// #ifdef _DEBUG
//     glEnable(GL_DEBUG_OUTPUT);
//     glDebugMessageCallback(MessageCallback, 0);
// #endif

//     glViewport(0, 0, width, height);
//     glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

//     glfwSetCursorPosCallback(window, windowMouseCallback);
//     glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

//     currentWindowPtr = this;
// }

// Window::~Window()
// {
//     glfwDestroyWindow(window);
//     glfwTerminate();
// }

// void Window::loop(std::function<void()>&& commands)
// {
//     while (!glfwWindowShouldClose(window)) {
//         commands();

//         glfwSwapBuffers(window);
//         glfwPollEvents();
//     }
// }

// void Window::setFrameBufferCallback(std::function<void(int width, int height)>&& fn)
// {
//     frameBufferFn = std::move(fn);
// }

// void Window::setMouseCallback(std::function<void(double x, double y)>&& fn)
// {
//     mouseCallbackFn = std::move(fn);
// }

// void Window::framebufferSizeCallback(UNUSED GLFWwindow* _window, int width, int height)
// {
//     glViewport(0, 0, width, height);
//     if (currentWindowPtr != nullptr && currentWindowPtr->frameBufferFn != nullptr) {
//         currentWindowPtr->width = width;
//         currentWindowPtr->height = height;
//         currentWindowPtr->frameBufferFn(width, height);
//     }
// }

// void Window::windowMouseCallback(UNUSED GLFWwindow* _window, double x, double y)
// {
//     if (currentWindowPtr != nullptr && currentWindowPtr->mouseCallbackFn != nullptr) {
//         currentWindowPtr->mouseCallbackFn(x, y);
//     }
// }

} // namespace Renderer
