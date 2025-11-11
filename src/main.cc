#include <alpm.h>
#include <curl/curl.h>
#include <unistd.h>

#include "app/app.hh"
#include "cli/cli.hh"
#include "log.hh"
#include "utils.hh"


auto
main(int p_argc, char **p_argv) -> int
{
    logger.set_log_level(utils::get_env("LOG_LEVEL"))
        .set_log_file(utils::get_env("LOG_FILE"));

#ifndef NDEBUG
    logger.log<INFO>("Running application in debug mode.");
#endif

    if (getuid() == 0)
    {
        auto cli { cli::Cli::init(p_argc, p_argv) };
        return cli->run();
    }

    /* Initializes version values in utils::VERSIONS */
    utils::init_versions();
    app::App app {};
    return app.run();
}
