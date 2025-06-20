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
#include <regex>

#include <gtkmm/widget.h>
#include <json/reader.h>
#include <json/value.h>
#include <curl/curl.h>

#include "utils.hh"
#include "types.hh"


namespace utils::str {
    auto
    split(str_view str, size_t pos) -> std::array<::str, 2>
    {
        const ::str first(str.substr(0, pos));
        const ::str second(str.substr(pos + 1));
        return { first , second };
    }


    auto
    trim(str_view str) -> ::str
    {
        size_t begin = str.find_first_not_of(" \t\n\r\f\v");
        if (begin == str_view::npos) {
            return "";
        }

        size_t end = str.find_last_not_of(" \t\n\r\f\v");
        return ::str { str.substr(begin, end - begin + 1) };
    }


    auto
    count(str_view str, char delim) -> usize
    {
        usize i = 0;
        for (auto s : str) {
            if (s == delim) i++;
        }

        return i;
    }


    auto
    is_digit(const ::str &str) -> bool
    {
        return std::ranges::all_of(str, [](char c){
            return std::isdigit(c);
        });
    }
} /* namespace utils::str */

namespace utils {
using std::strlen;

auto run_command(const ::str &cmd, usize buffer_size)
    -> optional<pair<::str, i32>> {
  vec<char> buffer(buffer_size);
  ::str result;

  auto pipe = std::unique_ptr<FILE, std::function<void(FILE *)>>(
      popen((cmd + " 2>&1").c_str(), "r"), [](FILE *f) {
        if (f)
          pclose(f);
      });

  if (!pipe)
    return std::nullopt;
  while (fgets(buffer.data(), static_cast<i32>(buffer.size()), pipe.get()) !=
         nullptr)
    result += buffer.data();

  return pair(result, pclose(pipe.release()));
}


    auto
    term_has_colors(i32 threshold_color_amount) -> bool
    {
        if (::str(std::getenv("COLORTERM")) == "truecolor") {
            return true;
        }

        auto result = run_command("tput colors 2> /dev/null");
        if (result != std::nullopt && result->second == EXIT_SUCCESS) {
            return (std::stoi(result->first) >= threshold_color_amount);
        }
        return false;
    }


    auto
    write_callback(void  *contents,
                   usize  size,
                   usize  nmemb,
                   ::str &userp) -> usize
    {
        usize total_size = size * nmemb;
        userp.append(static_cast<char*>(contents), total_size);
        return total_size;
    }


    auto
    perform_curl(CURL        *curl,
                 const ::str &url,
                 ::str       &read_buffer) -> CURLcode
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
    execvp(::str      &file,
           vec<::str> &argv) -> i32
    {
        vec<char*> c_argv;
        c_argv.reserve(argv.size() + 2);

        c_argv.push_back(file.data());

        for (::str &arg : argv) {
            c_argv.push_back(arg.data());
        }
        c_argv.push_back(nullptr);

        return ::execvp(file.c_str(), c_argv.data());
    }


    auto
    serrno() -> ::str
    {
        auto err = errno;
        return strerror(err);
    }


    auto
    get_env(const ::str &name) -> ::str
    {
        const char *env = std::getenv(name.c_str());

        if (env == nullptr) return "";
        return env;
    }


    auto
    sort_json(
        const json  &root,
        const ::str &sort_by,
        bool         reverse
    ) -> vec<std::reference_wrapper<const json>>
    {
        vec<std::reference_wrapper<const json>> result;
        result.reserve(root.size());

        for (const auto &j : root) {
            if (j.isMember(sort_by)) {
                result.emplace_back(j);
            }
        }

        std::ranges::sort(result, [&](const auto &a_ref, const auto &b_ref) {
            const auto &a = a_ref.get();
            const auto &b = b_ref.get();

            const auto &a_val = a[sort_by];
            const auto &b_val = b[sort_by];

            if (a_val.isInt() && b_val.isInt()) {
                /* I dont know why but the goddamn sort will reverse
                 * if the reverse is false...
                 */
                return reverse
                    ? a_val.asInt64() < b_val.asInt64()
                    : a_val.asInt64() > b_val.asInt64();
            }

            return reverse
                ? a_val.asString() < b_val.asString()
                : a_val.asString() > b_val.asString();
        });

        return result;
    }


    auto
    expand_envs(const ::str &str) -> ::str
    {
        ::str output = str;

        /* Expand ~ to $HOME */
        if (!output.empty() && output[0] == '~') {
            ::str home = get_env("HOME");
            if (!home.empty()) output.replace(0, 1, home);
        }

        std::regex env_var_regex(R"(\$([A-Za-z_][A-Za-z0-9_]*))");
        std::smatch match;

        ::str::const_iterator search_start(output.cbegin());
        while (std::regex_search(search_start,
                                 output.cend(),
                                 match,
                                 env_var_regex)) {
            ::str env_val = get_env(match[1].str());

            if (env_val.empty()) {
                search_start = match.suffix().first;
                continue;
            }

            output.replace(match.position(0), match.length(0), env_val);
            search_start = output.cbegin() +
                           match.position(0) +
                           static_cast<i32>(env_val.length());
        }

        return output;
    }


    auto
    get_ui_file(const fs::path &file_name,
                const ::str    &base_path) -> ::str
    {
        namespace fs = std::filesystem;

        auto valid_file = [](const fs::path &file_path) -> bool {
            return fs::exists(file_path)
                && fs::is_regular_file(file_path);
        };

        std::array<fs::path, 13> candidates = {
            fs::path("package") / file_name,
            fs::path("icons") / file_name,
            fs::path(".") / file_name,
            fs::path(base_path) / file_name,
            fs::path(base_path) / "ui/package" / file_name,
            fs::path(base_path) / "ui/icons" / file_name,
            fs::path(base_path) / "ui" / file_name,
            fs::path("ui/package") / file_name,
            fs::path("ui/icons") / file_name,
            fs::path("ui") / file_name,
            fs::path("/usr/share") / APP_NAME / "ui/package" / file_name,
            fs::path("/usr/share") / APP_NAME / "ui/icons" / file_name,
            fs::path("/usr/share") / APP_NAME / "ui" / file_name
        };

        for (const fs::path &p : candidates) {
            if (valid_file(p)) return fs::canonical(p);
        }
        return "";
    }
    } /* namespace utils */