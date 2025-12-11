#pragma once
#include <optional>


namespace cli
{
    class Cli
    {
    public:
        [[nodiscard]]
        static auto init(const int &argc, char **argv)
            -> std::optional<Cli>;


        auto run() -> int;

    private:
        Cli(const int &argc, char **argv, bool &err);
    };
}
