#pragma once
#include <gtkmm/box.h>

#include "app/tab.hh"


namespace Gtk
{
    class ToggleButton;
    class Builder;
    class Button;
    class Image;
    class Box;
}


namespace app
{
    /* Provides a sidebar showing the queues that the tabs contain. */
    class Sidebar : public Gtk::Box
    {
    public:
        Sidebar(BaseObjectType                   *p_object,
                const Glib::RefPtr<Gtk::Builder> &p_builder,
                Gtk::ToggleButton                *p_sidebar_toggle);


        void add_queue_signal(Tab::signal_signature_queue p_signal);

    private:
        Gtk::ToggleButton *m_sidebar_toggle;

        Gtk::Button *m_install_button;
        Gtk::Box    *m_container;

        Gtk::Image *m_toggle_icon;

        std::map<std::string, Gtk::Box *> m_queue_boxes;

    protected:
        void on_button_toggle();


        void on_queue_mutation(const std::string              &p_tab_name,
                               std::map<std::string, Package> &p_package_queue);
    };
}
