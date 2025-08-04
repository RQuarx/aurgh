#pragma once
#include <format>
#include <string>
#include <print>

#define TOSTRING( x ) std::string(#x)
#define M_ASSERT( condition, ... )             \
    _m_assert(condition,                       \
              TOSTRING(condition),             \
              __PRETTY_FUNCTION__,             \
              __FILE__, __LINE__ __VA_OPT__(,) \
              __VA_ARGS__)

constexpr static std::string_view RED   { "\033[31m" };
constexpr static std::string_view BOLD  { "\033[1m"  };
constexpr static std::string_view RESET { "\033[0m"  };


template<typename... T_Args>
constexpr void _m_assert( bool               condition,
                          const std::string &condition_str,
                          const std::string &func,
                          const std::string &file,
                          int                line,
                          std::format_string<T_Args...> fmt,
                          T_Args &&...args  )
{
    if (condition) return;

    std::string msg { std::format(fmt, std::forward<T_Args>(args)...) };
    std::println(stderr,
                "Assertion {}`{}` {}failed{} at function `{}{} in {}:{}`{}",
                 BOLD, condition_str, RED, RESET,
                 BOLD, func, file, line, RESET);
    std::println(stderr, "  what(): {}{}{}", BOLD, msg, RESET);
    std::abort();
}


using str_pair = std::pair<std::string, std::string>;
/**
 * @brief Splits a string at @p idx .
 *
 * @param str The string to split.
 * @param idx The index where the string will be split.
 * @return An std::pair<std::string, std::string> instance.
 *
 * The first string of the pair will contain the
 *   0 -> idx part of the string,
 * and the second string will contain
 *  idx + 1 -> str.length() part of the string.
 */
auto split_string( const std::string &str, const size_t &idx ) -> str_pair;


/**
 * @brief Returns the PREFIX_PATH from config.hh
 */
auto get_prefix_path( void ) -> std::string;