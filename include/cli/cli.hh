#pragma once
#include <optional>


namespace cli
{
    class Cli
    {
    public:
        [[nodiscard]]
        static auto init(const int &p_argc, char **p_argv)
            -> std::optional<Cli>;


        auto run() -> int;

    private:
        Cli(const int &p_argc, char **p_argv, bool &p_err);
    };
}
