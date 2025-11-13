#pragma once
#include <gtkmm.h>

#include "app/sidebar.hh"
#include "app/tabs/aur.hh"

namespace Gtk
{
    class ToggleButton;
    class Box;
}


namespace app
{
    class App
    {
    public:
        App();


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

        std::unique_ptr<app::Sidebar> sidebar;
        Gtk::Box                     *sidebar_box;

    protected:
        static void init_curl(std::int64_t p_flags);
        static void load_css();

        void setup_tabs(this App &self);
        void setup_criteria();

        void on_tab_button_pressed(Gtk::ToggleButton *p_button);
        void on_criteria_change(CriteriaType p_type,
                                TabType      p_tab = TabType::NONE);
    };
}
