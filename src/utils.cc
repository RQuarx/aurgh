#include <algorithm>
#include <chrono>
#include <format>
#include <gtkmm-3.0/gtkmm/label.h>
#include "utils.hh"

using std::chrono::duration;
using std::chrono::milliseconds;


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
} /* namespace Str */


namespace Utils {
    auto
    get_current_time() -> std::string
    {
        duration now = std::chrono::system_clock::now().time_since_epoch();

        auto milliseconds =
            std::chrono::duration_cast<std::chrono::milliseconds>(now) % 1000;
        auto minutes =
            std::chrono::duration_cast<std::chrono::minutes>(now) % 60;
        auto seconds = now % 60;

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
    term_has_colors(size_t threshold_color_amount) -> bool
    {
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
} /* namespace GtkUtils */