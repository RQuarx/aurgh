#pragma once
#include <gtkmm/application.h>

#include "app/dialog.hh"
#include "app/sidebar.hh"
#include "app/tabs/aur.hh"

namespace Gtk
{
    class ToggleButton;
    class Box;
}


namespace app
{
    /**
     * Main application class for AURGH.
     *
     * [note]---------------------------
     *
     * Handles GTK initialization, window setup, tab management,
     * sidebar, search criteria, and the main GTK event loop.
     */
    class App
    {
    public:
        App();


        /**
         * Run the application main loop.
         *
         * [returns]---------------------
         *
         * Exit code from the GTK main loop.
         */
        auto run() -> int;

    private:
        Glib::RefPtr<Gtk::Application> m_app;

        Gtk::Window *m_window;

        Gtk::ToggleButton *m_sidebar_button;
        Gtk::ToggleButton *m_aur_button;
        Gtk::ToggleButton *m_main_button;
        Gtk::ToggleButton *m_installed_button;

        CriteriaWidgets m_criteria;

        Gtk::Box *m_content_box;

        aur::Tab *m_aur_tab;
        Gtk::Box *m_main_tab;
        Gtk::Box *m_installed_tab;

        std::unique_ptr<app::Sidebar> m_sidebar;
        Gtk::Box                     *m_sidebar_box;

    protected:
        /* Initialize the CURL library. */
        static void init_curl(std::int64_t p_flags);


        /* Load the application CSS from resources. */
        static void load_css();


        /* Load and configure application tabs from XML builders. */
        void setup_tabs();


        /* Connect signals and default values for search/sort criteria. */
        void setup_criteria();


        /* Handle clicks on tab toggle buttons. */
        void on_tab_button_pressed(Gtk::ToggleButton *p_button);


        /**
         * Called when search/sort criteria are modified.
         *
         * [params]--------------------------------------
         *
         * `p_type`:
         *   The type of criteria that changed.
         *
         * `p_tab`:
         *   Which tab this change applies to,
         *   `TabType::NONE` means current tab.
         */
        void on_criteria_change(CriteriaType p_type,
                                TabType      p_tab = TabType::NONE);
    };
}
