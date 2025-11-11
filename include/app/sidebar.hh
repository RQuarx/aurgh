#pragma once
#include <gtkmm/box.h>
#include <gtkmm/togglebutton.h>


namespace app
{
    class Sidebar
    {
    public:
        Sidebar(Gtk::ToggleButton *sidebar_toggle, Gtk::Box *container);

    private:
        Gtk::ToggleButton *m_sidebar_toggle;
        Gtk::Box          *m_container;
    };
}
