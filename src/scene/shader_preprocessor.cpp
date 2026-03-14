#include "shader_preprocessor.hpp"

std::string get_lines_between_delims_inclusive(std::string_view string, std::string_view start, std::string_view end)
{
    std::string result;
    std::vector<std::string_view> lines;
    usize begin = 0;
    for (usize i = 0; i < string.size(); i++) {
        if (string[i] == '\n') {
            lines.emplace_back(&string.at(begin), i - begin);
            begin = i + 1;
        } else if (i == string.size() - 1) {
            lines.emplace_back(&string.at(begin), i - begin);
        }
    }

    begin = 0;
    bool matched_start = false;
    for (usize i = 0; i < lines.size(); i++) {
        if (lines[i] == start) {
            begin = i;
            matched_start = true;
        }
        if (lines[i] == end) {
            util_assert(matched_start == true, std::format("Failed to match \"{}\"", start));
            for (; begin <= i; begin++) {
                result += lines[begin];
                result += '\n';
            }
            return result;
        }
    }

    return result;
}

std::string get_lines_between_delims(std::string_view string, std::string_view start, std::string_view end)
{
    std::string result;
    std::vector<std::string_view> lines;
    usize begin = 0;
    for (usize i = 0; i < string.size(); i++) {
        if (string[i] == '\n') {
            lines.emplace_back(&string.at(begin), i - begin);
            begin = i + 1;
        } else if (i == string.size() - 1) {
            lines.emplace_back(&string.at(begin), i - begin);
        }
    }

    begin = 0;
    bool matched_start = false;
    for (usize i = 0; i < lines.size(); i++) {
        if (lines[i] == start) {
            begin = i + 1;
            matched_start = true;
        }
        if (lines[i] == end) {
            for (; begin < i; begin++) {
                result += lines[begin];
                result += '\n';
            }
            util_assert(matched_start == true, std::format("Failed to match \"{}\"", start));
            return result;
        }
    }

    return result;
}