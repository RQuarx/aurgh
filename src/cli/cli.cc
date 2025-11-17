#include <print>
#include <string>
#include <vector>

#include "cli/cli.hh"
#include "log.hh"

using enum LogLevel;
using cli::Cli;


auto
Cli::init(const int &p_argc, char **p_argv) -> std::optional<Cli>
{
    bool err { false };
    Cli  cli { p_argc, p_argv, err };
    if (err) return std::nullopt;
    return cli;
}


Cli::Cli(const int &p_argc, char **p_argv, bool &p_err)
{
    if (p_argc < 3)
    {
        logger.log<ERROR>("Not enough information passed");
        p_err = true;
        return;
    }

    std::string              job { p_argv[1] };
    std::vector<std::string> packages { p_argv + 2, p_argv + p_argc };

    std::print("{} {}: ", p_argc, job);
    for (const std::string &pkg : packages) { std::print("{} ", pkg); }
    std::println("");
}


auto
Cli::run() -> int
{
    return 1;
}
