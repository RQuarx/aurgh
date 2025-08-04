#pragma once
#include <unordered_map>
#include <optional>
#include <variant>
#include <string>
#include <vector>
#include <span>


class ArgParser
{
    using arg_input = std::pair<std::string, std::string>;
    using param_types = std::variant<std::string, int64_t, bool>;
public:
    ArgParser( std::span<char *> p_args );

    [[nodiscard]]
    auto get() -> const std::unordered_map<std::string, param_types> &;


    void add_flag( const arg_input &input );


    template<typename T>
    void add_option( const arg_input &input,
                     T                default_option )
    {
        static_assert(std::is_same_v<T, bool>
                   || std::is_same_v<T, std::string>
                   || std::is_integral_v<T>,
            "Expected either a string, bool, or an integral type.");

        std::string name { input.second.starts_with("--")
                         ? input.second.substr(2)
                         : input.second };

        if (args_contain_long(input.second) < 0 &&
            args_contain_short(input.first) < 0) {
            m_results.emplace(name, default_option);
            return;
        }

        T option { get_option_from_arg<T>(input, default_option) };
        m_results.emplace(name, option);
    }

private:
    std::unordered_map<std::string, param_types> m_results;
    std::vector<std::string> m_args;


    auto args_contain_short( const std::string &short_arg ) -> ssize_t;


    auto args_contain_long( const std::string &long_arg  ) -> ssize_t;


    template<typename T>
    auto get_option_from_arg( const arg_input &arg,
                              T                default_option ) -> T
    {
        const ssize_t s_idx { args_contain_short(arg.first) };
        const ssize_t l_idx { args_contain_long(arg.second) };
        std::optional<T> value;

        if (l_idx > -1) {
            value = to_type<T>(m_args.at(l_idx + 1));
            m_args.erase(m_args.begin() + l_idx + 1);
            m_args.erase(m_args.begin() + l_idx);

            return value.value_or(default_option);
        }

        if (s_idx == -1) return default_option;

        size_t s_len { m_args.at(s_idx).length() };
        if (s_len == 2) {
            value = to_type<T>(m_args.at(s_idx + 1));
            m_args.erase(m_args.begin() + s_idx + 1);
            m_args.erase(m_args.begin() + s_idx);

            return value.value_or(default_option);
        }

        char short_arg { arg.first.at(arg.first.size() - 1) };
        for (size_t i { 1 }; i < s_len; i++) {

            if (m_args.at(s_idx).at(i) == short_arg) {
                std::string substr { m_args.at(s_idx).substr(i + 1) };
                value = to_type<T>(substr);

                m_args.at(s_idx).erase(i);
                return value.value_or(default_option);
            }
        }

        return default_option;
    }


    template<typename T_Res>
    auto to_type( const std::string &str ) -> std::optional<T_Res>
    {
        if (str.empty()) return std::nullopt;

        if constexpr (std::is_same_v<T_Res, std::string>) return str;
        if constexpr (std::is_same_v<T_Res, bool>       ) return str == "true";
        if constexpr (std::is_integral_v<T_Res>         ) {
            try { return std::stoi(str); }
            catch (...) { return std::nullopt; }
        }

        return std::nullopt;
    }
};