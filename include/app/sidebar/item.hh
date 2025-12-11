#pragma once
#include <string>
#include <vector>


namespace Gtk
{
    class ToggleButton;
    class Label;
    class Box;
}


namespace app
{
    class SidebarItem
    {
    public:
        SidebarItem();


        auto set_name(const std::string &name) -> SidebarItem &;
        auto set_visible(bool visible) -> SidebarItem &;

        auto add_package(const std::string &package_name) -> SidebarItem &;
        auto pop_package(const std::string &package_name) -> SidebarItem &;

        auto get_container() -> Gtk::Box *;
        [[nodiscard]]
        auto is_visible() const -> bool;

    private:
        Gtk::Box *m_container;

        Gtk::ToggleButton *m_show_package_button;

        Gtk::Box                 *m_labels_container;
        std::vector<Gtk::Label *> m_package_name_labels;
    };
}
