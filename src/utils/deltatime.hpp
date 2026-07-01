#pragma once

namespace Utils {

class DeltaTime : public NoCopyNoMove {
public:
    DeltaTime();
    ~DeltaTime() = default;

    void reset();
    void update();

    template <typename T>
    [[nodiscard]] T delta_time() const
    {
        return static_cast<T>(m_delta_time);
    }

private:
    std::chrono::high_resolution_clock m_clock;
    std::chrono::system_clock::time_point m_start_time;
    std::chrono::system_clock::time_point m_current_time;
    double m_delta_time {};
};

} // namespace Utils
