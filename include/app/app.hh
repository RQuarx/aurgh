#pragma once
#include <gtkmm.h>

#include "app/tabs/aur.hh"
#include "app/types.hh"

namespace Gtk
{
    class ToggleButton;
    class Box;
}


namespace app
{
    /**
     * @brief A class that manages the app's backend and frontend.
     */
    class App
    {
    public:
        App();


        /* Runs the application. */
        auto run(this App &self) -> int;

    private:
        Glib::RefPtr<Gtk::Application> app;

        Gtk::Window *window;

        Gtk::ToggleButton *sidebar_button;
        Gtk::ToggleButton *aur_button;
        Gtk::ToggleButton *main_button;
        Gtk::ToggleButton *installed_button;

        CriteriaWidgets criteria;

        Gtk::Box *content_box;

        aur::Tab *aur_tab;
        Gtk::Box *main_tab;
        Gtk::Box *installed_tab;

        signal_type signal;

    protected:
        /**
         * @brief Initialize libCURL.
         *
         * @param p_flags Initialization flag that will be passed to curl_global_init.
         * @return Expected nothing, and an error message on error.
         */
        static auto init_curl(int64_t p_flags)
            -> std::expected<void, std::string>;


        /**
         * @brief Loads custom CSS theme from {app-dir}/ui/style.css
         *
         * @throw This function may throw an std::runtime_error with no messages.
         */
        static void load_css();

        void on_tab_button_pressed(Gtk::ToggleButton *p_button);
        void setup_tabs(this App &self);
        void setup_criteria();
        void on_criteria_change(CriteriaType p_type, TabType p_tab = TAB_NONE);
    };
}
