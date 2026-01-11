#pragma once

#include <print>

namespace Utils {

static constexpr bool runtime_checks = true;
static constexpr bool use_throws = true;

template <typename StrType>
constexpr void util_assert(bool error, StrType error_msg, const char* file, int line)
{
    if constexpr (runtime_checks) {
        if (!error) {
            std::string msg = std::format("{}:{}: {}", file, line, error_msg);
            if constexpr (use_throws) {
                throw std::runtime_error(msg);
            } else {
                std::println(msg);
                std::quick_exit(EXIT_FAILURE);
            }
        }
    }
}

#define util_assert(error, msg) \
    Utils::util_assert(error, msg, __FILE__, __LINE__)

} // namespace Utils