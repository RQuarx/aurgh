#include <string>
#include <vector>
#include <print>

#include "cli/cli.hh"
#include "log.hh"

using cli::Cli;


auto
Cli::init( const std::shared_ptr<Logger> &p_logger,
           const int32_t                 &p_argc,
           char                         **p_argv
         ) -> std::optional<Cli>
{
    bool err { false };
    Cli cli { p_logger, p_argc, p_argv, err };
    if (err) return std::nullopt;
    return cli;
}


Cli::Cli( const std::shared_ptr<Logger> &p_logger,
          const int32_t                 &p_argc,
          char                         **p_argv,
          bool                          &p_err ) :
    m_logger(p_logger)
{
    if (p_argc < 3) {
        m_logger->log<ERROR>("Not enough information passed.");
        p_err = true;
        return;
    }

    std::string job { p_argv[1] };
    std::vector<std::string> packages { p_argv + 2, p_argv + p_argc };

    std::print("{} {}: ", p_argc, job);
    for (const std::string &pkg : packages) {
        std::print("{} ", pkg);
    }
    std::println("");
}