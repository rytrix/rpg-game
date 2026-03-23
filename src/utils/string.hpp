#pragma once

namespace Utils {

// A staticly sized string
class String {
public:
    explicit String(const char* string);
    explicit String(const std::string& string);
    explicit String(const std::string_view string);

    [[nodiscard]] const char* cstr() const;
    [[nodiscard]] std::string_view view() const;
    [[nodiscard]] const char* data() const;
    [[nodiscard]] usize size() const;

    bool operator==(String other) const;
    bool operator==(std::string_view other) const;
    bool operator==(const char* other) const;

private:
    static constexpr usize SIZE = 255;
    char m_data[SIZE + 1] {};
    usize m_size = 0;
};

} // namespace Utils

template <>
struct std::hash<Utils::String> {
    std::size_t operator()(Utils::String const& string) const noexcept
    {
        return std::hash<std::string_view> {}(string.view());
    }
};
