#include <gtkmm.h>

#include "app/sidebar/item.hh"
#include "logger.hh"

using app::SidebarItem;


SidebarItem::SidebarItem()
    : m_container(Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 5)),
      m_show_package_button(Gtk::make_managed<Gtk::ToggleButton>()),
      m_labels_container(
          Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 0))
{
    m_container->set_valign(Gtk::ALIGN_START);

    m_show_package_button->set_valign(Gtk::ALIGN_START);
    m_show_package_button->set_halign(Gtk::ALIGN_FILL);
    m_show_package_button->set_margin_bottom(5);
    m_show_package_button->set_margin_start(5);
    m_show_package_button->set_margin_top(5);
    m_show_package_button->set_margin_end(5);

    m_labels_container->set_valign(Gtk::ALIGN_START);
    m_labels_container->set_halign(Gtk::ALIGN_FILL);
    m_labels_container->set_margin_start(30);

    m_container->pack_start(*m_show_package_button);
    m_container->pack_start(*m_labels_container);

    m_container->show_all_children();
    m_container->set_visible(false);
    m_labels_container->set_visible(false);

    m_show_package_button->signal_toggled().connect(
        [this]() -> void
        {
            logger[Level::TRACE, "app::sidebar"](
                "Package group \"{}\" is {}",
                m_show_package_button->get_label().raw(),
                m_show_package_button->get_active() ? "shown" : "hidden");

            m_labels_container->set_visible(
                m_show_package_button->get_active());
        });
}


auto
SidebarItem::set_name(const std::string &name) -> SidebarItem &
{
    m_show_package_button->set_label(name);
    m_show_package_button->get_child()->set_halign(Gtk::ALIGN_START);
    return *this;
}


auto
SidebarItem::set_visible(bool visible) -> SidebarItem &
{
    m_container->set_visible(visible);
    return *this;
}


auto
SidebarItem::add_package(const std::string &package_name) -> SidebarItem &
{
    auto *label { Gtk::make_managed<Gtk::Label>() };

    label->set_text(package_name);
    label->set_halign(Gtk::ALIGN_START);
    label->set_valign(Gtk::ALIGN_START);
    label->set_visible(true);

    m_labels_container->pack_start(*label);
    m_package_name_labels.emplace_back(label);

    return *this;
}


auto
SidebarItem::pop_package(const std::string &package_name) -> SidebarItem &
{
    auto it { std::ranges::find_if(
        m_package_name_labels, [&package_name](Gtk::Label *label) -> bool
        { return label->get_label() == package_name; }) };

    if (it != m_package_name_labels.end())
    {
        m_labels_container->remove(**it);
        m_package_name_labels.erase(it);
    }

    if (m_package_name_labels.empty()) set_visible(false);

    return *this;
}


auto
SidebarItem::get_container() -> Gtk::Box *
{
    return m_container;
}


auto
SidebarItem::is_visible() const -> bool
{
    return m_container->is_visible();
}
