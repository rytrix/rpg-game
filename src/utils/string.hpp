#pragma once

namespace Utils {

// A staticly sized string
class String {
public:
    String() = default;
    String(const char* string);
    String(const std::string& string);
    String(const std::string_view string);

    String& operator=(const char* string);
    String& operator=(const std::string& string);
    String& operator=(const std::string_view string);

    [[nodiscard]] const char* cstr() const;
    [[nodiscard]] std::string_view view() const;
    [[nodiscard]] char* data();
    [[nodiscard]] const char* data() const;
    [[nodiscard]] usize size() const;
    [[nodiscard]] usize capacity() const;

    bool operator==(String other) const;
    bool operator==(std::string_view other) const;
    bool operator==(const char* other) const;

    void set_size(usize size);
    void clear();

    template <typename... Args>
    String& format(std::format_string<Args...> fmt, Args&&... args);

private:
    static constexpr usize SIZE = 255;
    char m_data[SIZE + 1] {};
    usize m_size = 0;
};

template <typename... Args>
String& String::format(std::format_string<Args...> fmt, Args&&... args)
{
    clear();
    auto [out, size] = std::format_to_n(m_data, capacity(), fmt, std::forward<Args>(args)...);
    m_size = std::min((usize)size, SIZE);
    util_assert(m_size != 0, "String::format returned size = 0");
    return *this;
}

} // namespace Utils

template <>
struct std::hash<Utils::String> {
    std::size_t operator()(Utils::String const& string) const noexcept
    {
        return std::hash<std::string_view> {}(string.view());
    }
};
