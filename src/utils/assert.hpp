#pragma once

#include <print>
#include <source_location>

namespace Utils {

static constexpr bool runtime_checks = true;
static constexpr bool use_throws = false;

template <typename StrType>
[[noreturn]] constexpr void util_error(StrType error_msg, const std::source_location location = std::source_location::current())
{
    std::string msg = std::format("{}:{},{} {} {}", location.file_name(), location.line(), location.column(), location.function_name(), error_msg);
    if constexpr (use_throws) {
        throw std::runtime_error(msg);
    } else {
        std::println(stderr, "{}", msg);
        std::quick_exit(EXIT_FAILURE);
    }
}

template <typename StrType>
constexpr void util_assert(bool error, StrType error_msg, std::source_location location = std::source_location::current())
{
    if constexpr (runtime_checks) {
        if (!error) {
            util_error(error_msg, location);
        }
    }
}

#define util_assert(error, msg) \
    Utils::util_assert(error, msg)

#define util_error(msg) \
    Utils::util_error(msg)

} // namespace Utils
