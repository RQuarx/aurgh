#pragma once
#include <filesystem>
#include <fstream>

#include "result.hh"
#include "utils.hh"


namespace aurgh::ini
{
    struct callback_data
    {
        const char      *filepath;
        std::string_view section;
        std::string_view key;
        std::string_view value;
        std::size_t      line_num;
    };


    template <typename F>
    auto
    parse(const std::filesystem::path &file,
          F                          &&callback,
          std::string                  section_override = {}) noexcept -> result<void>
        requires std::is_invocable_v<F, callback_data>
    {
        std::ifstream stream { file };

        if (!stream.good())
            return error { "failed to open config file ({})", file.c_str() }.unexpected();

        std::string section  = std::move(section_override);
        std::size_t line_num = 0;

        callback_data data { .filepath = file.c_str(), .section = section }; /* NOLINT */

        for (std::string line; std::getline(stream, line); line_num++)
        {
            std::string_view trimmed { line | views::trim };

            if (trimmed.empty() or trimmed.starts_with('#')) continue;
            data.key      = {};
            data.value    = {};
            data.line_num = line_num;

            if (trimmed.starts_with('[') and trimmed.ends_with(']'))
            {
                section      = trimmed.substr(1, trimmed.length() - 2);
                data.section = section;

                if (aurgh::result<void> res = callback(data); !res.has_value())
                    return res.error().unexpected();
                continue;
            }


            if (auto it = trimmed.find('='); it == std::string_view::npos) /* no '=' */
                data.key = trimmed;
            else
            {
                data.key   = std::string_view { trimmed.substr(0, it) | views::trim };
                data.value = std::string_view { trimmed.substr(it + 1) | views::trim };
            }

            if (aurgh::result<void> res = callback(data); !res.has_value())
                return res.error().unexpected();
        }

        if (!stream and !stream.eof())
            return error { "failed to read config file ({})", file.c_str() }.unexpected();
        return {};
    }
}
