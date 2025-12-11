#include <gtkmm.h>

#include "app/sidebar/widget.hh"
#include "logger.hh"

using app::Sidebar;


Sidebar::Sidebar(BaseObjectType                   *object,
                 const Glib::RefPtr<Gtk::Builder> &builder,
                 Gtk::ToggleButton                *sidebar_toggle)
    : Gtk::Box(object), m_sidebar_toggle(sidebar_toggle),
      m_toggle_icon(dynamic_cast<Gtk::Image *>(m_sidebar_toggle->get_child()))
{
    m_sidebar_toggle->signal_clicked().connect(
        sigc::mem_fun(*this, &Sidebar::on_button_toggle));

    builder->get_widget("aurgh_sidebar_install_button", m_install_button);
    builder->get_widget("aurgh_sidebar_container", m_container);

    set_size_request(270);
}


void
Sidebar::add_queue_signal(Tab::signal_signature_queue signal)
{
    signal.connect(sigc::mem_fun(*this, &Sidebar::on_queue_mutation));
}


void
Sidebar::on_button_toggle()
{
    const bool  activated { m_sidebar_toggle->get_active() };
    const char *icon_name { activated ? "pan-start-symbolic"
                                      : "pan-end-symbolic" };

    logger[Level::TRACE, "app::sidebar"]("Sidebar is {}",
                                         activated ? "shown" : "hidden");

    m_toggle_icon->set_from_icon_name(icon_name, Gtk::ICON_SIZE_BUTTON);
    this->set_visible(activated);
}


void
Sidebar::on_queue_mutation(const std::string  &tab_name,
                           const std::string  &package_name,
                           Tab::QueueOperation operation)
{
    /* Higher level function that calls this function already logs the
       operations, no need to log here.
    */

    if (operation == Tab::QueueOperation::POP)
    {
        m_package_groups[tab_name].pop_package(package_name);

        if (!std::ranges::all_of(m_package_groups, [](const auto &kv) -> bool
                                 { return kv.second.is_visible(); }))
        {
            m_sidebar_toggle->set_visible(false);
            set_visible(false);
        }
    }
    else
    {
        m_sidebar_toggle->set_visible(true);

        if (m_sidebar_toggle->get_active()) set_visible(true);

        bool contains { m_package_groups.contains(tab_name) };

        m_package_groups[tab_name].add_package(package_name).set_visible(true);

        if (!contains)
        {
            m_package_groups[tab_name].set_name(tab_name);
            m_container->pack_start(
                *m_package_groups.at(tab_name).get_container());
        }
    }
}

