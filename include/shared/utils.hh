#pragma once
#include <charconv>
#include <expected>
#include <ranges>
#include <string_view>


namespace aurgh
{
    namespace util
    {
        template <typename T, auto (*D)(T *)>
        struct destructor
        {
            void
            operator()(T *obj) noexcept
            {
                if (obj != nullptr) D(obj);
            }
        };


        template <std::integral T>
        [[nodiscard]]
        auto
        to_integral(const char *first, const char *end) noexcept -> std::expected<T, std::errc>
        {
            T val;
            if (auto res = std::from_chars(first, end, val); res.ec != std::errc {})
                return std::unexpected { res.ec };
            return val;
        }
    }


    namespace views
    {
        namespace _impl
        {
            [[nodiscard]]
            constexpr auto
            is_space(char c) noexcept -> bool
            {
                using namespace std::string_view_literals;
                return " \r\n\f\v\t"sv.contains(c);
            }


            struct trim final : std::ranges::range_adaptor_closure<trim>
            {
                constexpr auto
                operator()(std::ranges::viewable_range auto &&r) const
                {
                    auto first = std::ranges::find_if_not(r, is_space);
                    auto last  = std::ranges::find_if_not(
                                     std::ranges::reverse_view {
                                         std::ranges::subrange { first, std::ranges::end(r) }
                    },
                                     is_space)
                                     .base();

                    return std::ranges::subrange { first, last };
                }
            };
        }

        inline constexpr _impl::trim trim {};
    }
}
