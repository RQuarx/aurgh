#include <unistd.h>

#include <curl/curl.h>
#include <alpm.h>

#include "app/tabs/aur.hh"
#include "app/app.hh"
#include "utils.hh"
#include "log.hh"


auto
main( int32_t p_argc, char **p_argv ) -> int
{
    if (getuid() == 0) {
    }

    utils::init_versions();
    auto logger { std::make_shared<Logger>(utils::get_env("LOG_LEVEL"),
                                           utils::get_env("LOG_FILE")) };
    app::App app { logger };
    return app.run();
}