#pragma once

namespace Renderer {

class Window {
public:
    Window(const char* name, int width, int height);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&) = default;
    Window& operator=(Window&&) = default;

    void process_input_callback(const std::function<void(SDL_Event& event)>& commands);
    void loop(const std::function<void()>& commands);

    static void capture_mouse(bool value);

    [[nodiscard]] SDL_Window* get_window_ptr() const;
    [[nodiscard]] std::pair<int, int> get_size() const;
    [[nodiscard]] std::pair<f32, f32> get_size_f32() const;
    [[nodiscard]] f32 get_aspect_ratio() const;
    void set_should_close();

private:
    int m_width;
    int m_height;
    SDL_GLContext m_context {};
    SDL_Window* m_window = nullptr;
    bool m_should_close = false;
    std::function<void(SDL_Event& event)> m_process_input_fn = nullptr;

    void process_input_internal();
};

} // namespace Renderer
