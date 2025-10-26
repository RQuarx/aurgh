#pragma once
#include <memory>

#include <gtkmm.h>

#include "app/tabs/aur.hh"
#include "app/types.hh"

namespace Gtk
{
    class ToggleButton;
    class Box;
}

class Logger;


namespace app
{
    /**
     * @brief A class that manages the app's backend and frontend.
     */
    class App
    {
    public:
        App(const std::shared_ptr<Logger> &p_logger);


        /* Runs the application. */
        auto run() -> int32_t;

    private:
        Glib::RefPtr<Gtk::Application> m_app;
        std::shared_ptr<Logger>        m_logger;

        Gtk::Window *m_window;

        Gtk::ToggleButton *m_aur_button;
        Gtk::ToggleButton *m_main_button;
        Gtk::ToggleButton *m_installed_button;

        CriteriaWidgets m_criteria;

        Gtk::Box *m_content_box;

        aur::Tab *m_aur_tab;
        Gtk::Box *m_main_tab;
        Gtk::Box *m_installed_tab;

        signal_type m_signal;

    protected:
        /**
         * @brief Initialize libCURL.
         *
         * @param p_flags Initialization flag that will be passed to curl_global_init.
         * @return Expected nothing, and an error message on error.
         */
        auto init_curl(int64_t p_flags) const
            -> std::expected<void, std::string>;


        /**
         * @brief Loads custom CSS theme from {app-dir}/ui/style.css
         *
         * @throw This function may throw an std::runtime_error with no messages.
         */
        void load_css() const;

        void on_tab_button_pressed(Gtk::ToggleButton *p_button);
        void setup_tabs();
        void setup_criteria();
        void on_criteria_change(CriteriaType p_type, TabType p_tab = TAB_NONE);
    };
}
