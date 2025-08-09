#include <print>

#include <unistd.h>

#include <curl/curl.h>
#include <alpm.h>

#include "app/tabs/aur.hh"
#include "arg_parser.hh"
#include "app/app.hh"
#include "log.hh"

#define GTKMM_STRING_VERSION                     \
    std::format("{}.{}.{}", GTKMM_MAJOR_VERSION, \
                            GTKMM_MINOR_VERSION, \
                            GTKMM_MICRO_VERSION)


namespace
{
    [[noreturn]]
    void
    print_version_info( void )
    {
        std::string top { std::format("{} {}", APP_NAME, APP_VERSION) };
        std::println("{}", top);
        std::println("{}", std::string(top.length(), '-'));
        std::println("  gtkmm   {}", GTKMM_STRING_VERSION);
        std::println("  libcurl {}", curl_version_info(
                                     CURLVERSION_NOW)->version);
        std::println("  jsoncpp {}", JSONCPP_VERSION_STRING);
        std::println("  libalpm {}", alpm_version());

        exit(0);
    }
}


auto
main( int32_t p_argc, char **p_argv ) -> int
{
    ArgParser arg_parser { std::span(p_argv, static_cast<size_t>(p_argc)) };

    arg_parser.add_option<std::string>({ "-l", "--log"    }, "warn");
    arg_parser.add_option<std::string>({ "-p", "--prefix" }, ""    );

    arg_parser.add_flag({ "-h", "--help"    });
    arg_parser.add_flag({ "-V", "--version" });

    const auto &args { arg_parser.get() };

    if (*std::get_if<bool>(&args.at("version"))) print_version_info();

    if (getuid() == 0) {
    }

    std::string logger_arg { *std::get_if<std::string>(&args.at("log")) };
    auto logger { std::make_shared<Logger>(logger_arg) };

    app::App app { logger };

    return app.run();
}