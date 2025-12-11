#include <print>
#include <string>
#include <vector>

#include "cli/cli.hh"
#include "logger.hh"

using cli::Cli;


auto
Cli::init(const int &argc, char **argv) -> std::optional<Cli>
{
    bool err { false };
    Cli  cli { argc, argv, err };
    if (err) return std::nullopt;
    return cli;
}


Cli::Cli(const int &argc, char **argv, bool &err)
{
    if (argc < 3)
    {
        logger[Level::ERROR, "cli"]("Not enough information passed");
        err = true;
        return;
    }

    std::string              job { argv[1] };
    std::vector<std::string> packages { argv + 2, argv + argc };

    std::print("{} {}: ", argc, job);
    for (const std::string &pkg : packages) { std::print("{} ", pkg); }
    std::println("");
}


auto
Cli::run() -> int
{
    return 1;
}
