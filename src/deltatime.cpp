#include "deltatime.hpp"

DeltaTime::DeltaTime()
{
    m_current_time = m_clock.now();
    update();
}

[[nodiscard]] float DeltaTime::delta_time() const
{
    return m_delta_time;
}

void DeltaTime::update()
{
    m_start_time = m_current_time;
    m_current_time = m_clock.now();
    m_delta_time = std::chrono::duration<float, std::chrono::seconds::period>(m_current_time - m_start_time).count();
}