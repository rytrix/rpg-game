#pragma once

namespace Utils {

class DeltaTime {
public:
    DeltaTime();
    ~DeltaTime() = default;

    DeltaTime(const DeltaTime&) = delete;
    DeltaTime& operator=(const DeltaTime&) = delete;
    DeltaTime(DeltaTime&&) = default;
    DeltaTime& operator=(DeltaTime&&) = default;

    void update();
    [[nodiscard]] float delta_time() const;

private:
    std::chrono::high_resolution_clock m_clock;
    std::chrono::system_clock::time_point m_start_time;
    std::chrono::system_clock::time_point m_current_time;
    float m_delta_time {};
};

} // namespace Utils
