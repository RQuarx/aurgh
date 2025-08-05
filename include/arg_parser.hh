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


    void add_flag( const arg_input &p_input );


    template<typename T>
    void add_option( const arg_input &p_input,
                     T                p_default )
    {
        static_assert(std::is_same_v<T, bool>
                   || std::is_same_v<T, std::string>
                   || std::is_integral_v<T>,
            "Expected either a string, bool, or an integral type.");

        std::string name { p_input.second.starts_with("--")
                         ? p_input.second.substr(2)
                         : p_input.second };

        if (args_contain_long(p_input.second) < 0 &&
            args_contain_short(p_input.first) < 0) {
            m_results.emplace(name, p_default);
            return;
        }

        T option { get_option_from_arg<T>(p_input, p_default) };
        m_results.emplace(name, option);
    }

private:
    std::unordered_map<std::string, param_types> m_results;
    std::vector<std::string> m_args;


    auto args_contain_short( const std::string &p_short ) -> ssize_t;


    auto args_contain_long( const std::string &p_long  ) -> ssize_t;


    template<typename T>
    auto get_option_from_arg( const arg_input &p_arg,
                              T                p_default ) -> T
    {
        const ssize_t s_idx { args_contain_short(p_arg.first) };
        const ssize_t l_idx { args_contain_long(p_arg.second) };
        std::optional<T> value;

        if (l_idx > -1) {
            value = to_type<T>(m_args.at(l_idx + 1));
            m_args.erase(m_args.begin() + l_idx + 1);
            m_args.erase(m_args.begin() + l_idx);

            return value.value_or(p_default);
        }

        if (s_idx == -1) return p_default;

        size_t s_len { m_args.at(s_idx).length() };
        if (s_len == 2) {
            value = to_type<T>(m_args.at(s_idx + 1));
            m_args.erase(m_args.begin() + s_idx + 1);
            m_args.erase(m_args.begin() + s_idx);

            return value.value_or(p_default);
        }

        char short_arg { p_arg.first.at(p_arg.first.size() - 1) };
        for (size_t i { 1 }; i < s_len; i++) {

            if (m_args.at(s_idx).at(i) == short_arg) {
                std::string substr { m_args.at(s_idx).substr(i + 1) };
                value = to_type<T>(substr);

                m_args.at(s_idx).erase(i);
                return value.value_or(p_default);
            }
        }

        return p_default;
    }


    template<typename T_Res>
    auto to_type( const std::string &p_str ) -> std::optional<T_Res>
    {
        if (p_str.empty()) return std::nullopt;

        if constexpr (std::is_same_v<T_Res, std::string>)
            return p_str;

        if constexpr (std::is_same_v<T_Res, bool>)
            return p_str == "true";

        if constexpr (std::is_integral_v<T_Res>) try {
            return std::stoi(p_str);
        } catch (...) { return std::nullopt; }

        return std::nullopt;
    }
};