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
#include <chrono>
#include <format>

#include <gtkmm/widget.h>
#include <json/reader.h>
#include <json/value.h>
#include <curl/curl.h>

#include "utils.hh"


using std::chrono::duration;
using ms = std::chrono::milliseconds;
using m = std::chrono::minutes;
using s = std::chrono::seconds;


namespace Str {
    auto
    splitp(std::string_view str, size_t pos) -> std::array<std::string, 2>
    {
        const std::string first(str.substr(0, pos));
        const std::string second(str.substr(pos + 1));
        return { first , second };
    }


    auto
    splitd(const std::string &str, char delim) -> std::vector<std::string>
    {
        std::vector<std::string> tokens;
        tokens.reserve(count(str, delim) + 1);

        std::istringstream iss(str);
        std::string        token;

        while (std::getline(iss, token, delim)) {
            tokens.push_back(token);
        }

        return tokens;
    }


    auto
    is_digit(std::string_view str) -> bool
    {
        return std::ranges::all_of(str, [](const char c){
            return std::isdigit(c);
        });
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
    count(std::string_view str, char c) -> size_t
    {
        size_t i = 0;
        for (auto s : str) {
            if (s == c) i++;
        }

        return i;
    }
} /* namespace Str */


namespace Utils {
    auto
    get_current_time() -> std::string
    {
        duration now = std::chrono::system_clock::now().time_since_epoch();

        ms milliseconds  = std::chrono::duration_cast<ms>(now) % 1000;
        m minutes        = std::chrono::duration_cast<m>(now) % 60;
        duration seconds = now % 60;

        return std::format(
            "{:02}:{:02}.{:03}",
            minutes.count(), seconds.count(), milliseconds.count()
        );
    }


    auto
    run_command(
        const std::string &cmd, size_t buffer_size
    ) -> std::optional<std::pair<std::string, int32_t>>
    {
        std::vector<char> buffer(buffer_size);
        std::string       result;

        std::unique_ptr<FILE, decltype(&pclose)> pipe(
            popen((cmd + " 2>&1").c_str(), "r"), pclose
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
        void *contents, size_t size, size_t nmemb, std::string &userp
    ) -> size_t
    {
        size_t total_size = size * nmemb;
        userp.append(static_cast<char*>(contents), total_size);
        return total_size;
    }


    auto
    perform_curl(
        CURL *curl, const std::string &url, std::string &read_buffer
    ) -> CURLcode
    {
        bool self_curl = (curl == nullptr);

        if (self_curl)       curl = curl_easy_init();
        if (curl == nullptr) return CURLE_FAILED_INIT;

        read_buffer.clear();
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Utils::write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &read_buffer);
        CURLcode res = curl_easy_perform(curl);

        if (self_curl) curl_easy_cleanup(curl);
        return res;
    }


    auto
    get_quote(size_t max_len) -> std::string
    {
        CURL *curl = curl_easy_init();
        if (curl == nullptr) return R"({"quote": "Unable to fetch quotes"})";

        std::string  read_buffer;
        Json::Reader reader;
        Json::Value  val;
        std::string  quote;

        while (quote.length() > max_len || quote.empty()) {
            CURLcode res = perform_curl(
                curl, "https://quotes-api-self.vercel.app/quote", read_buffer);

            if (res != CURLE_OK) {
                curl_easy_cleanup(curl);
                return R"({"quote": "Failed to fetch quote"})";
            }

            if (!reader.parse(read_buffer, val)) {
                curl_easy_cleanup(curl);
                return R"({"quote": "Failed to parse response"})";
            }

            quote = val["quote"].asString();
        }

        curl_easy_cleanup(curl);
        return quote;
    }


    auto
    execvp(
        std::string &file, std::vector<std::string> &argv
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
} /* namespace Utils */


namespace GtkUtils {
    void
    set_margin(Gtk::Widget &widget, std::array<int32_t, 4> margin)
    {
        widget.set_margin_top(margin.at(0));
        widget.set_margin_right(margin.at(1));
        widget.set_margin_bottom(margin.at(2));
        widget.set_margin_left(margin.at(3));
    }

    void
    set_margin(Gtk::Widget &widget, int32_t margin_y, int32_t margin_x)
    {
        widget.set_margin_top(margin_y);
        widget.set_margin_right(margin_x);
        widget.set_margin_bottom(margin_y);
        widget.set_margin_left(margin_x);
    }

    void
    set_margin(Gtk::Widget &widget, int32_t margin)
    {
        widget.set_margin_top(margin);
        widget.set_margin_right(margin);
        widget.set_margin_bottom(margin);
        widget.set_margin_left(margin);
    }
} /* namespace GtkUtils */