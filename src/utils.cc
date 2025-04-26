/**
 * @file utils.cc
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
#include <chrono>
#include <format>
#include <gtkmm-3.0/gtkmm/image.h>
#include <gtkmm-3.0/gtkmm/label.h>
#include <gtkmm-3.0/gtkmm/box.h>
#include "utils.hh"

using std::chrono::duration;
using ms = std::chrono::milliseconds;
using m = std::chrono::minutes;
using s = std::chrono::seconds;


namespace Str {
    auto
    split(std::string_view str, size_t pos) -> std::array<std::string, 2>
    {
        const std::string first(str.substr(0, pos));
        const std::string second(str.substr(pos + 1));
        return { first , second };
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
} /* namespace Str */


namespace Utils {
    auto
    get_current_time() -> std::string
    {
        duration now = std::chrono::system_clock::now().time_since_epoch();

        ms milliseconds = std::chrono::duration_cast<ms>(now) % 1000;
        m minutes = std::chrono::duration_cast<m>(now) % 60;
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
        std::string result;

        std::unique_ptr<FILE, decltype(&pclose)> pipe(
            popen((cmd + " 2>&1").c_str(), "r"), pclose
        );

        if (!pipe) return std::nullopt;
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }

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
    set_margin(Gtk::Widget &widget, std::array<int32_t, 2> margin)
    {
        widget.set_margin_top(margin.at(0));
        widget.set_margin_right(margin.at(1));
        widget.set_margin_bottom(margin.at(0));
        widget.set_margin_left(margin.at(1));
    }

    void
    set_margin(Gtk::Widget &widget, int32_t margin)
    {
        widget.set_margin_top(margin);
        widget.set_margin_right(margin);
        widget.set_margin_bottom(margin);
        widget.set_margin_left(margin);
    }

    auto
    create_label_markup(const std::string &markup) -> Gtk::Label*
    {
        auto *label = Gtk::make_managed<Gtk::Label>();
        label->set_markup(markup);
        return label;
    }

    auto
    create_label_icon(
        const std::string &icon,
        const std::string &markup,
        Gtk::IconSize icon_size
    ) -> Gtk::Box*
    {
        Gtk::Label *label = create_label_markup(markup);
        label->set_margin_left(5);

        auto *image = Gtk::make_managed<Gtk::Image>();
        image->set_from_icon_name(icon, icon_size);

        auto *box = Gtk::make_managed<Gtk::Box>();
        box->pack_start(*image);
        box->pack_start(*label);
        return box;
    }
} /* namespace GtkUtils */