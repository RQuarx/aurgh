#pragma once
#include <expected>
#include <format>
#include <source_location>


namespace aurgh
{
    namespace _impl
    {
        template <typename... Args>
        struct source_format_string
        {
            std::format_string<Args...> fmt;
            std::source_location        src;


            template <typename T>
            constexpr source_format_string(const T                    &fmt,
                                           const std::source_location &src
                                           = std::source_location::current())
                requires std::constructible_from<std::format_string<Args...>, T>
                : fmt { fmt }, src { src }
            {
            }
        };


        template <typename... Args>
        using source_format_string_identity = std::type_identity_t<source_format_string<Args...>>;


        struct to_unexpected
        {
            template <typename T>
            [[nodiscard]]
            auto
            unexpected(this T &self) noexcept -> std::unexpected<T>
            { return std::unexpected { std::move(self) }; }


            template <typename T>
            [[nodiscard]]
            auto
            unexpected(this const T &self) noexcept -> std::unexpected<T>
            { return std::unexpected { self }; }
        };
    }


    class error : public _impl::to_unexpected
    {
    public:
        template <typename... Args>
        error(_impl::source_format_string_identity<Args...> fmt, Args &&...args)
            : m_message { std::format(fmt.fmt, std::forward<Args>(args)...) }, m_source { fmt.src }
        {
        }


        [[nodiscard]]
        auto
        message() const noexcept -> std::string_view
        { return m_message; }


        [[nodiscard]]
        auto
        source() const noexcept -> std::source_location
        { return m_source; }

    private:
        std::string          m_message;
        std::source_location m_source;
    };


    template <typename T>
    using result = std::expected<T, error>;
}
