#pragma once
#include <memory>
#include <optional>

class Logger;


namespace cli
{
    class Cli
    {
    public:
        [[nodiscard]]
        static auto init(const std::shared_ptr<Logger> &p_logger,
                         const int32_t                 &p_argc,
                         char **p_argv) -> std::optional<Cli>;


        auto run() -> int;

    private:
        std::shared_ptr<Logger> m_logger;

        Cli(const std::shared_ptr<Logger> &p_logger,
            const int32_t                 &p_argc,
            char                         **p_argv,
            bool                          &p_err);
    };
}
