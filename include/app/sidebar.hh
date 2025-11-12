#pragma once
#include <gtkmm/box.h>
#include <gtkmm/togglebutton.h>


namespace app
{
    class Sidebar
    {
    public:
        Sidebar(Gtk::ToggleButton *p_sidebar_toggle, Gtk::Box *p_container);

    private:
        Gtk::ToggleButton *sidebar_toggle;
        Gtk::Box          *container;

        Gtk::Image *toggle_icon;

    protected:
        void on_button_toggle();
    };
}
