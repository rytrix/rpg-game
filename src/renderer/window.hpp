#pragma once

namespace Renderer {

class Window : public NoCopyNoMove {
public:
    Window() = default;
    Window(const char* name, int width, int height);
    ~Window();

    void init(const char* name, int width, int height);

    void process_input_callback(const std::function<void(SDL_Event& event)>& commands);
    void loop(const std::function<void()>& commands);

    void set_capture_mouse(bool value);
    void set_relative_mode(bool value);

    [[nodiscard]] SDL_Window* get_window_ptr() const;
    [[nodiscard]] std::pair<int, int> get_size() const;
    [[nodiscard]] int get_width() const;
    [[nodiscard]] int get_height() const;
    [[nodiscard]] std::pair<f32, f32> get_size_f32() const;
    [[nodiscard]] f32 get_aspect_ratio() const;
    void set_should_close();
    void set_window_title(const char* title);

private:
    bool initialized = false;

    int m_width {};
    int m_height {};
    SDL_GLContext m_context {};
    SDL_Window* m_window = nullptr;
    bool m_should_close = false;
    std::function<void(SDL_Event& event)> m_process_input_fn = nullptr;

    void process_input_internal();
};

} // namespace Renderer
