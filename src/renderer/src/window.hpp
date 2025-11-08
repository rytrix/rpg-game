#pragma once

namespace Renderer {

// struct Window {
//     Window(const char* name, int width, int height);
//     ~Window();

//     void loop(std::function<void()>&& commands);
//     void setFrameBufferCallback(std::function<void(int width, int height)>&& fn);
//     void setMouseCallback(std::function<void(double x, double y)>&& fn);
//     [[nodiscard]] bool shouldClose() { return glfwWindowShouldClose(window); };
//     void swapBuffers() { glfwSwapBuffers(window); }
//     void pollEvents() { glfwPollEvents(); }

//     NODISCARD GLFWwindow* getWindowPtr() const { return window; }
//     NODISCARD std::pair<int, int> getSize() const { return { width, height }; }

// private:
//     int width;
//     int height;
//     GLFWwindow* window = nullptr;
//     std::function<void(int width, int height)> frameBufferFn = nullptr;
//     std::function<void(double x, double y)> mouseCallbackFn = nullptr;

//     static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
//     static void windowMouseCallback(GLFWwindow* _window, double x, double y);
// };

} // namespace Renderer
