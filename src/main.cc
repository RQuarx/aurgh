#include <cstring>
#include <print>

#include "app/app.hh"
#include "cli/cli.hh"
#include "logger.hh"
#include "versions.hh"


namespace
{
    void
    glib_logger(const gchar   *log_domain,
                GLogLevelFlags log_level,
                const gchar   *message,
                gpointer /* unused */)
    {
        std::string domain { log_domain == nullptr ? "GLib" : log_domain };

        logger[Logger::GLogLevel_to_Level(log_level), domain]("{}", message);
        if ((log_level & G_LOG_LEVEL_ERROR) != 0) std::terminate();
    }


    void
    glib_print(const gchar *message)
    {
        logger[Level::INFO, "stdio"]("{}", message);
    }

    void
    glib_printerr(const gchar *message)
    {
        logger[Level::INFO, "stderr"]("{}", message);
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
main(int argc, char **argv) -> int
{
    for (int i { 1 }; i < argc; i++)
        if (std::strcmp(argv[i], "version") == 0)
        {
            std::println("{} {}", APP_NAME, versions::get(APP_NAME));

            for (const auto &[name, ver] : versions::get())
            {
                if (name == APP_NAME) continue;
                std::println("  {:<10} {}", name, ver);
            }

            return 0;
        }

    set_glib_logger();

#ifdef NDEBUG
    logger[Level::INFO, "main"]("Running {} version {}", APP_NAME,
                                versions::get(APP_NAME));
#else
    logger[Level::INFO, "main"]("Running {}-debug version {}", APP_NAME,
                                versions::get(APP_NAME));
#endif

    if (getuid() == 0)
    {
        auto cli { cli::Cli::init(argc, argv) };
        return cli->run();
    }

    app::App app {};
    return app.run();
}
