/**
 * aurgh Copyright (C) 2025 RQuarx
 *
 * This file is part of aurgh
 *
 * aurgh is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * aurgh is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with aurgh. If not, see <https://www.gnu.org/licenses/>.
 */

#include <algorithm>
#include <cerrno>
#include <print>
#include <regex>

#include <gtkmm/widget.h>
#include <json/reader.h>
#include <json/value.h>
#include <curl/curl.h>

#include "utils.hh"


namespace str {
    auto
    split(std::string_view str, size_t pos) -> std::array<std::string, 2>
    {
        const std::string first(str.substr(0, pos));
        const std::string second(str.substr(pos + 1));
        return { first , second };
    }


    auto
    trim(std::string_view str) -> std::string
    {
        size_t begin = str.find_first_not_of(" \t\n\r\f\v");
        if (begin == std::string_view::npos) {
            return "";
        }

        size_t end = str.find_last_not_of(" \t\n\r\f\v");
        return std::string{str.substr(begin, end - begin + 1)};
    }


    auto
    count(std::string_view str, char delim) -> size_t
    {
        size_t i = 0;
        for (auto s : str) {
            if (s == delim) i++;
        }

        return i;
    }


    auto
    is_digit(const std::string &str) -> bool
    {
        return std::ranges::all_of(str, [](char c){
            return std::isdigit(c);
        });
    }
} /* namespace str */


namespace utils {
    auto
    run_command(
        const std::string &cmd,
        size_t             buffer_size
    ) -> optional<std::pair<std::string, int32_t>>
    {
        std::vector<char> buffer(buffer_size);
        std::string       result;

        auto pipe = std::unique_ptr<FILE, std::function<void(FILE*)>>(
            popen((cmd + " 2>&1").c_str(), "r"),
            [](FILE* f) { if (f) pclose(f); }
        );

        if (!pipe) return std::nullopt;
        while (fgets(
                buffer.data(), static_cast<int32_t>(buffer.size()), pipe.get()
            ) != nullptr
        ) result += buffer.data();

        return std::pair(result, pclose(pipe.release()));
    }


    auto
    term_has_colors(int32_t threshold_color_amount) -> bool
    {
        if (std::string(std::getenv("COLORTERM")) == "truecolor") {
            return true;
        }

        auto result = run_command("tput colors 2> /dev/null");
        if (result != std::nullopt && result->second == EXIT_SUCCESS) {
            return (std::stoi(result->first) >= threshold_color_amount);
        }
        return false;
    }


    auto
    write_callback(
        void        *contents,
        size_t       size,
        size_t       nmemb,
        std::string &userp
    ) -> size_t
    {
        size_t total_size = size * nmemb;
        userp.append(static_cast<char*>(contents), total_size);
        return total_size;
    }


    auto
    perform_curl(
        CURL              *curl,
        const std::string &url,
        std::string       &read_buffer
    ) -> CURLcode
    {
        bool self_curl = (curl == nullptr);

        if (self_curl)       curl = curl_easy_init();
        if (curl == nullptr) return CURLE_FAILED_INIT;

        read_buffer.clear();
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, utils::write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &read_buffer);
        CURLcode res = curl_easy_perform(curl);

        if (self_curl) curl_easy_cleanup(curl);
        return res;
    }


    auto
    execvp(
        std::string              &file,
        std::vector<std::string> &argv
    ) -> int32_t
    {
        std::vector<char*> c_argv;
        c_argv.reserve(argv.size() + 2);

        c_argv.push_back(file.data());

        for (std::string &arg : argv) {
            c_argv.push_back(arg.data());
        }
        c_argv.push_back(nullptr);

        return ::execvp(file.c_str(), c_argv.data());
    }


    auto
    serrno() -> std::string
    {
        auto err = errno;
        return strerror(err);
    }


    auto
    get_env(const std::string &name) -> std::string
    {
        const char *env = std::getenv(name.c_str());

        if (env == nullptr) return "";
        return env;
    }


    auto
    sort_json(
        const Json::Value &root,
        const std::string &sort_by,
        bool               reverse
    ) -> Json::Value
    {
        Json::Value              result{ root.type() };
        std::vector<Json::Value> buffer;
        buffer.reserve(root.size());

        for (const auto &j : root) { buffer.emplace_back(j); }

        std::ranges::sort(buffer,
        [&sort_by, reverse]
        (const Json::Value &a, const Json::Value &b){
            if (a[sort_by].isInt()) {
                if (reverse) {
                    return a[sort_by].asInt64() < b[sort_by].asInt64();
                }
                return a[sort_by].asInt64() > b[sort_by].asInt64();
            }

            if (reverse) {
                return a[sort_by].asString() < b[sort_by].asString();
            }
            return a[sort_by].asString() > b[sort_by].asString();
        });

        for (const auto &j : buffer) { result.append(j); }
        return result;
    }


    auto
    expand_envs(const std::string &str) -> std::string
    {
        std::regex           envar_regex { R"(\$([A-Za-z_][A-Za-z0-9_]*))" };
        std::sregex_iterator it          { str.begin(), str.end(), envar_regex };
        std::sregex_iterator end;
        std::string          result;
        size_t               last_pos = 0;

        for (; it != end; it++) {
            const std::smatch &match     = *it;
            size_t             match_pos = match.position();
            size_t             match_len = match.length();
            std::string        var_name  = match.str(1);
            std::string        value     = utils::get_env(var_name);

            result += str.substr(last_pos, match_pos - last_pos);
            result += (value.empty() ? match.str(1) : value );

            last_pos = match_pos + match_len;
        }

        result += str.substr(last_pos);
        return result;
    }
} /* namespace utils */