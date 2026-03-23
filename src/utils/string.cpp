#include "string.hpp"

namespace Utils {

String::String(const char* string)
{
    usize len = strlen(string);
    util_assert(len < SIZE, std::format("String size of {} overflowed by const char* string of {}", SIZE, len));
    m_size = len;
    memcpy(m_data, string, len);
}

String::String(const std::string& string)
{
    usize len = string.size();
    util_assert(len < SIZE, std::format("String size of {} overflowed by std::string of {}", SIZE, len));
    m_size = len;
    memcpy(m_data, string.data(), len);
}

String::String(const std::string_view string)
{
    usize len = string.size();
    util_assert(len < SIZE, std::format("String size of {} overflowed by std::string_view of {}", SIZE, len));
    m_size = len;
    memcpy(m_data, string.data(), len);
}

const char* String::cstr() const
{
    return m_data;
}

std::string_view String::view() const
{
    return { m_data, m_size };
}

const char* String::data() const
{
    return m_data;
}

usize String::size() const
{
    return m_size;
}

bool String::operator==(String other) const
{
    if (m_size == other.m_size) {
        if (memcmp(m_data, other.m_data, m_size) == 0) {
            return true;
        }
    }
    return false;
}

bool String::operator==(std::string_view other) const
{
    if (m_size == other.size()) {
        if (memcmp(m_data, other.data(), m_size) == 0) {
            return true;
        }
    }
    return false;
}

bool String::operator==(const char* other) const
{
    if (m_size == strlen(other)) {
        if (memcmp(m_data, other, m_size) == 0) {
            return true;
        }
    }
    return false;
}

} // namespace Utils
