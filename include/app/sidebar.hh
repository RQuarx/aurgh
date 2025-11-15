#pragma once

namespace Gtk
{
    class ToggleButton;
    class Image;
    class Box;
}


namespace app
{
    /* Provides a sidebar showing the queues that the tabs contain. */
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
