#include <cstring>
#include <print>

#include "app/app.hh"
#include "cli/cli.hh"
#include "log.hh"
#include "utils.hh"
#include "versions.hh"

using enum LogLevel;


namespace
{
    void
    glib_logger(const gchar   *log_domain,
                GLogLevelFlags log_level,
                const gchar   *message,
                gpointer /* unused */)
    {
        std::string_view domain { log_domain == nullptr ? "GLib" : log_domain };

        logger.glog(GLogLevel_to_LogLevel(log_level), domain, "{}", message);
        if ((log_level & G_LOG_LEVEL_ERROR) != 0) std::terminate();
    }


    void
    glib_print(const gchar *message)
    {
        logger.glog(INFO, "stdio", "{}", message);
    }

    void
    glib_printerr(const gchar *message)
    {
        logger.glog(INFO, "stderr", "{}", message);
    }


    void
    set_glib_logger()
    {
        g_log_set_default_handler(glib_logger, nullptr);
        g_set_print_handler(glib_print);
        g_set_printerr_handler(glib_printerr);
    }
}


auto
main(int p_argc, char **p_argv) -> int
{
    for (int i { 1 }; i < p_argc; i++)
        if (std::strcmp(p_argv[i], "version") == 0)
        {
            std::println("{} {}", APP_NAME, versions::get(APP_NAME));

            for (const auto &[name, ver] : versions::get())
            {
                if (name == APP_NAME) continue;
                std::println("  {:<10} {}", name, ver);
            }
            std::exit(0);
        }

    logger.set_log_level(utils::get_env("LOG_LEVEL"))
        .set_log_file(utils::get_env("LOG_FILE"));

    set_glib_logger();

    logger.log<INFO>("Running {} version {}", APP_NAME,
                     versions::get(APP_NAME));

#ifndef NDEBUG
    logger.log<DEBUG>("Running application in debug mode.");
#endif

    if (getuid() == 0)
    {
        auto cli { cli::Cli::init(p_argc, p_argv) };
        return cli->run();
    }

    app::App app {};
    return app.run();
}
