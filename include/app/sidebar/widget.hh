#pragma once
#include <gtkmm/box.h>

#include "app/sidebar/item.hh"
#include "app/tab.hh"


namespace Gtk
{
    class TreeStore;
    class TreeView;
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
        using PopPkgSignature = std::function<void(const std::string &)>;


        Sidebar(BaseObjectType                   *object,
                const Glib::RefPtr<Gtk::Builder> &builder,
                Gtk::ToggleButton                *sidebar_toggle);


        void add_queue_signal(Tab::signal_signature_queue signal);

    private:
        Gtk::ToggleButton *m_sidebar_toggle;

        Gtk::Button *m_install_button;
        Gtk::Box    *m_container;
        Gtk::Image  *m_toggle_icon;

        std::unordered_map<std::string, app::SidebarItem> m_package_groups;

    protected:
        void on_button_toggle();


        void on_queue_mutation(const std::string  &tab_name,
                               const std::string  &package_name,
                               Tab::QueueOperation operation);
    };
}
