#include "arg_parser.hh"


ArgParser::ArgParser( std::span<char *> p_args )
{
    for (std::string arg : p_args) {
        if (arg.starts_with("--")) {
            if (!arg.contains('=')) {
                m_args.push_back(arg);
                continue;
            }

            size_t eq_idx = arg.find('=');

            m_args.push_back(arg.substr(0,  eq_idx));
            m_args.push_back(arg.substr(eq_idx + 1));
            continue;
        }

        if (arg.starts_with('-')) {
            if (!arg.contains('=')) {
                m_args.push_back(arg);
                continue;
            }

            size_t eq_idx = arg.find('=');

            m_args.push_back(arg.substr(0,  eq_idx));
            m_args.push_back(arg.substr(eq_idx + 1));
        }
    }
}


void
ArgParser::add_flag( const arg_input &p_input )
{
    std::string name { p_input.second.starts_with("--")
                     ? p_input.second.substr(2)
                     : p_input.second };

    ssize_t s_idx { args_contain_short(p_input.first) };
    ssize_t l_idx { args_contain_long(p_input.second) };
    bool exist { s_idx > -1 || l_idx > -1 };
    param_types value { exist };

    m_results.emplace(name, value);

    if (l_idx > -1) {
        m_args.erase(m_args.begin() + l_idx);
    } else if (s_idx > -1) {
        char short_arg { p_input.first.at(p_input.first.size() - 1) };

        m_args.at(s_idx).erase(m_args.at(s_idx).find(short_arg));
    }
}


auto
ArgParser::args_contain_short( const std::string &p_short ) -> ssize_t
{
    if (p_short.empty()) return -1;
    const bool clean { !p_short.starts_with('-') };

    for (size_t i { 0 }; i < m_args.size(); i++) {
        std::string &arg = m_args.at(i);

        if (!arg.starts_with("--") && !arg.starts_with('-')) continue;

        if (!clean) {
            if (arg.contains(p_short.at(1))) return i;
        } else if (arg.contains(p_short)) return i;
    }

    return -1;
}


auto
ArgParser::args_contain_long( const std::string &p_long ) -> ssize_t
{
    const bool clean { !p_long.starts_with("--") };

    for (size_t i { 0 }; i < m_args.size(); i++) {
        const std::string &arg { m_args.at(i) };

        if (!arg.starts_with("--")) continue;

        if (!clean) {
            if (p_long == arg) return i;
        } else if (p_long == arg.substr(2)) return i;
    }

    return -1;
}


auto
ArgParser::get() -> const std::unordered_map<std::string, param_types> &
{ return m_results; }