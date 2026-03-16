#include "deltatime.hpp"

namespace Utils {

DeltaTime::DeltaTime()
{
    m_current_time = m_clock.now();
    update();
}

void DeltaTime::update()
{
    m_start_time = m_current_time;
    m_current_time = m_clock.now();
    m_delta_time = std::chrono::duration<double, std::chrono::seconds::period>(m_current_time - m_start_time).count();
}

} // namespace Utils