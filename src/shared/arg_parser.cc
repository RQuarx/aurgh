#include <algorithm>
#include <cassert>
#include <print>

#include "arg_parser.hh"


ArgParser::ArgParser(int32_t argc, char **argv) :
    m_bin_path(*argv),
    m_override_help(false),
    m_override_version(false)
{
    m_arg_list[Short].reserve(argc);
    m_arg_list[Long].reserve(argc);

    Type prev_type = Short;
    for (int32_t i = 1; i < argc; i++) {
        sv arg = argv[i];

        if (arg.starts_with("--")) {
            m_arg_list[Long].emplace_back(false, arg.substr(2));
            prev_type = Long;
            continue;
        }

        if (arg.starts_with('-')) {
            m_arg_list[Short].emplace_back(false, arg.substr(1));
            prev_type = Short;
            continue;
        }

        m_arg_list[prev_type].emplace_back(true, arg);
    }
}


void
ArgParser::parse()
{
    if (!m_override_help && find_arg({ "-h", "--help" })) {
        std::println(
            "\033[1m\033[4mUsage:\033[0m {} <options {{param}}>\n", m_bin_path);
        std::println("\033[1m\033[4mOptions:\033[0m");

        for (const auto &arg : m_defined_args) {
            std::println("{}", arg);
        }

        exit(EXIT_SUCCESS);
    }

    if (!m_override_version && find_arg({ "-V", "--version" })) {
        std::println("{} - {}", APP_NAME, APP_VERSION);
        exit(EXIT_SUCCESS);
    }

    std::string                               str_args;
    std::vector<std::pair<bool, std::string>> args;

    for (auto t : { Short, Long }) {
        if (m_arg_list.at(t).empty()) continue;

        args.insert(
            args.begin(), m_arg_list.at(t).begin(), m_arg_list.at(t).end()
        );
    }

    if (args.empty()) return;

    for (size_t i = 0; i < args.size(); i++) {
        auto &[option, arg] = args.at(i);

        if (!option) {
            str_args += std::format(
                "{}{}-{}",
                str_args.empty() ? "" : ", ",
                i <= m_arg_list.at(Short).size() ? "" : "-",
                arg
            );
        } else {
            str_args += std::format(" {}", arg);
        }
    }

    throw std::invalid_argument(std::format(
        "Non recognised argument{} detected: [ {} ]",
        m_arg_list.at(Short).size() == 1 ? "" : "s", str_args
    ));
}


auto
ArgParser::get_flag(const std::string &name) -> bool
{
    if (!m_user_arg.contains(name)) return false;
    auto *val = std::get_if<bool>(&m_user_arg.at(name));

    return (val != nullptr) && *val;
}


auto
ArgParser::get_option(const std::string &name) -> std::string
{
    if (!m_user_arg.contains(name)) return "";
    auto *val = std::get_if<std::string>(&m_user_arg.at(name));

    if (val == nullptr) return "";
    return *val;
}


auto
ArgParser::clean_arg(const arg_pair &arg) -> arg_pair
{
    arg_pair cleaned_arg;

    /* Remove a dash from short args (-X -> X) */
    cleaned_arg.first = arg.first.starts_with('-')
        ? arg.first.substr(1)
        : arg.first;

    /* Remove double-dash from long args (--arg -> arg) */
    cleaned_arg.second = arg.second.starts_with("--")
        ? arg.second.substr(2)
        : arg.second;

    return cleaned_arg;
}


auto
ArgParser::find_arg(arg_pair arg) -> bool
{
    arg = clean_arg(arg);

    if (m_arg_list.contains(Short)) {
        auto &vec = m_arg_list.at(Short);
        auto  it  = std::ranges::find_if(vec,
        [&](const auto &entry)
        { return !entry.first && entry.second.contains(arg.first); });

        if (it != vec.end()) {
            vec.erase(it);
            return true;
        }
    }

    if (!m_arg_list.contains(Long)) return false;

    auto &vec = m_arg_list.at(Long);
    auto  it  = std::ranges::find_if(vec,
    [&](const auto &entry)
    { return !entry.first && entry.second == arg.second; });

    if (it == vec.end()) return false;
    vec.erase(it);
    return true;
}


auto
ArgParser::option_arg(std::string &param, arg_pair arg) -> bool
{
    arg = clean_arg(arg);

    if (!find_option_short(param, arg.first)) {
        return find_option_long(param, arg.second);
    }
    return true;
}


auto
ArgParser::find_option_short(
    std::string &option, std::string_view short_arg
) -> bool
{
    if (!m_arg_list.contains(Short)) { return false; }
    if (short_arg.size() == 0)       { return false; }

    auto &vec      = m_arg_list.at(Short);
    const char arg = short_arg.at(0);

    for (auto it = vec.begin(); it != vec.end(); it++) {
        if (it->first) continue;

        /* Case: -X=value */
        if (it->second.contains('=')) {
            auto [flag, val] = split_str(it->second, it->second.find('='));

            if (flag.ends_with(arg)) {
                option = val;
                it->second.erase(flag.length() - 1);
                return true;
            }
        }

        /* Case: -Xvalue */
        if (it->second.length() > 1 && it->second.front() == arg) {
            option = it->second.substr(1);
            vec.erase(it);
            return true;
        }

        /* Case: -X value */
        for (auto c : it->second) {
            if (c == arg) {
                auto next = std::next(it);
                if (next != vec.end() && next->first) {
                    option = next->second;
                    vec.erase(next);
                    vec.erase(it);
                    return true;
                }
            }
        }
    }

    option.clear();
    return false;
}


auto
ArgParser::find_option_long(
    std::string &option, std::string_view long_arg
) -> bool
{
    if (!m_arg_list.contains(Long)) return false;

    auto &vec = m_arg_list.at(Long);
    for (auto it = vec.begin(); it != vec.end(); ++it) {
        if (it->first) continue;

        /* Case: --arg=value */
        if (it->second.contains('=')) {
            auto [flag, val] = split_str(it->second, it->second.find('='));
            if (flag == long_arg) {
                option = val;
                vec.erase(it);
                return true;
            }
        }

        /* Case: --arg value */
        if (it->second == long_arg) {
            auto next = std::next(it);
            if (next != vec.end() && next->first) {
                option = next->second;
                vec.erase(next);
                vec.erase(it);
                return true;
            }
        }
    }

    option.clear();
    return false;
}


auto
ArgParser::split_str(sv str, size_t pos) -> arg_pair
{
    std::string first  {str.substr(0, pos)};
    std::string second {str.substr(pos + 1)};
    return { first, second };
}