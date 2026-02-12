#pragma once

#include <fstream>

template <typename T>
constexpr std::vector<T> read_file(const char* path)
{
    std::vector<T> text;
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file) {
        LOG_ERROR(std::format("could not open file \"{}\"", path));
        return {};
    }

    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    text.reserve(size + 1);
    text.resize(size);
    file.read(text.data(), size);

    text.push_back('\0');

    return text;
}