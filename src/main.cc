#include <unistd.h>

#include "app/tabs/aur.hh"
#include "arg_parser.hh"
#include "app/app.hh"
#include "log.hh"


auto
main( int32_t p_argc, char **p_argv ) -> int
{
    ArgParser arg_parser { std::span(p_argv, static_cast<size_t>(p_argc)) };

    arg_parser.add_option<std::string>({ "-l", "--log"    }, "warn");
    arg_parser.add_option<std::string>({ "-p", "--prefix" }, ""    );

    arg_parser.add_flag({ "-h", "--help"    });
    arg_parser.add_flag({ "-V", "--version" });

    const auto &args { arg_parser.get() };

    if (getuid() == 0) {
    }

    std::string logger_arg { *std::get_if<std::string>(&args.at("log")) };
    auto logger { std::make_shared<Logger>(logger_arg) };

    app::App app { logger };
    return app.run();
}