#include <algorithm>
#include "arg_parser.hh"
#include "utils.hh"


ArgParser::ArgParser(int32_t argc, char **argv) :
    m_bin_path(argv[0])
{
    Type previous_type = Short;
    for (int32_t i = 1; i < argc; i++) {
        std::string_view arg = argv[i];

        if (arg.starts_with("--")) {
            m_arg_list[Long].emplace_back(false, arg.substr(2));
            previous_type = Long;
            continue;
        }

        if (arg.starts_with('-')) {
            m_arg_list[Short].emplace_back(false, arg.substr(1));
            previous_type = Short;
            continue;
        }

        m_arg_list[previous_type].emplace_back(true, arg);
    }
}


auto
ArgParser::clean_input_arg(const arg_pair &input_arg) -> arg_pair
{
    arg_pair cleaned_arg;

    /* Remove a dash from short args (-X -> X) */
    cleaned_arg.first = input_arg.first.starts_with('-')
        ? input_arg.first.substr(1)
        : input_arg.first;

    /* Remove double-dash from long args (--arg -> arg) */
    cleaned_arg.second = input_arg.second.starts_with("--")
        ? input_arg.second.substr(2)
        : input_arg.second;

    return cleaned_arg;
}


auto
ArgParser::find_arg(arg_pair arg) -> bool
{
    arg = clean_input_arg(arg);

    if (m_arg_list.contains(Short)) {
        for (const auto &a : m_arg_list.at(Short)) {
            if (!a.first && a.second.contains(arg.first)) return true;
        }
    }

    if (!m_arg_list.contains(Long)) return false;
    return std::ranges::any_of(m_arg_list.at(Long), [&arg](const auto &a){
        return !a.first && a.second == arg.second;
    });
}


auto
ArgParser::option_arg(std::string &option, arg_pair arg) -> bool
{
    arg = clean_input_arg(arg);

    return (
        find_option_short(option, arg.first)
        || find_option_long(option, arg.second)
    );
}


auto
ArgParser::find_option_short(std::string &option, std::string_view short_arg) -> bool
{
    if (!m_arg_list.contains(Short)) return false;

    char arg = short_arg.at(0);
    for (size_t i = 0; i < m_arg_list.at(Short).size(); i++) {
        const auto &a = m_arg_list.at(Short).at(i);
        if (a.first) continue;

        /* Case: -X=value */
        if (a.second.contains('=')) {
            auto arg_and_option = Str::split(a.second, a.second.find('='));

            if (arg_and_option.front().back() == arg) {
                option = arg_and_option.back();
                return true;
            }
        }

        /* Case: -Xvalue */
        if (a.second.length() > 1) {
            if (a.second.front() == arg) {
                option = a.second.substr(1);
                return true;
            }
        }

        /* Case: -X value */
        for (const auto &c : a.second) {
            if (c == arg && i + 1 < m_arg_list.at(Short).size()) {
                const auto &next_arg = m_arg_list.at(Short).at(i + 1);
                if (next_arg.first) {
                    option = next_arg.second;
                    return true;
                }
            }
        }
    }

    option.clear();
    return false;
}


auto
ArgParser::find_option_long(std::string &option, std::string_view long_arg) -> bool
{
    if (!m_arg_list.contains(Long)) return false;

    for (size_t i = 0; i < m_arg_list.at(Long).size(); i++) {
        const auto &a = m_arg_list.at(Long).at(i);
        if (a.first) continue;

        /* Case: --arg=value */
        if (a.second.contains('=')) {
            size_t eq_pos = a.second.find_first_of('=');

            std::pair<std::string_view, std::string_view> split_arg = {
                a.second.substr(0, eq_pos), a.second.substr(eq_pos + 1)
            };

            if (split_arg.first == long_arg.substr(2)) {
                option = split_arg.second;
                return true;
            }
        }

        /* Case: --arg value */
        if (a.second == long_arg.substr(2) && i + 1 < m_arg_list.at(Long).size()) {
            const auto &next_arg = m_arg_list.at(Long).at(i + 1);

            if (next_arg.first) {
                option = next_arg.second;
                return true;
            }
        }
    }

    option.clear();
    return false;
}


auto
ArgParser::back() -> std::string
{
    for (const auto &t : { Short, Long }) {
        if (!m_arg_list.contains(t)) continue;
        auto back = m_arg_list.at(t).back();
        if (back.first) return std::string(back.second);
    }

    return "";
}