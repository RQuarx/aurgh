#include <alpm.h>
#include <curl/curl.h>
#include <unistd.h>

#include "app/app.hh"
#include "app/tabs/aur.hh"
#include "cli/cli.hh"
#include "log.hh"
#include "utils.hh"


auto
main(int32_t p_argc, char **p_argv) -> int
{
    auto logger { std::make_shared<Logger>(utils::get_env("LOG_LEVEL"),
                                           utils::get_env("LOG_FILE")) };
#ifdef APP_DEBUG
    logger->log<INFO>("Running application in debug mode.");
#endif

    if (getuid() == 0)
    {
        auto cli { cli::Cli::init(logger, p_argc, p_argv) };
        return cli->run();
    }

    /* Initializes version values in utils::VERSIONS */
    utils::init_versions();
    app::App app { logger };
    return app.run();
}
