#include <gtkmm.h>

#include "app/sidebar/widget.hh"
#include "app/tab.hh"
#include "logger.hh"

using app::CriteriaWidgets;
using app::Tab;


auto
CriteriaWidgets::get_values() const
    -> std::tuple<std::string, std::string, std::string, bool>
{
    return std::make_tuple(search_by->get_active_id(), sort_by->get_active_id(),
                           search_entry->get_text(), reverse->get_active());
}


Tab::Tab(BaseObjectType                   *object,
         const Glib::RefPtr<Gtk::Builder> &builder,
         std::string                       tab_name,
         std::string                       domain)
    : Gtk::Box(object), m_spinner(std::make_unique<Gtk::Spinner>()),
      m_spinner_running(false), m_domain(std::move(domain)),
      m_tab_name(std::move(tab_name))
{
    builder->get_widget("content", m_content_box);

    m_spinner->set_halign(Gtk::ALIGN_FILL);
    m_spinner->set_valign(Gtk::ALIGN_CENTER);
    m_spinner->set_vexpand();
    m_spinner->set_visible();
}


auto
Tab::signal_queue_update() -> signal_signature_queue
{
    return m_queue_signal;
}


auto
Tab::get_tab_name() const -> std::string
{
    return m_tab_name;
}


void
Tab::push_pkg(PackageEntry pkg)
{
    logger[Level::INFO, m_domain]("Adding package {} to the \"{}\" queue",
                                  pkg[PackageField::NAME], m_tab_name);

    m_queue_signal.emit(m_tab_name, pkg[PackageField::NAME],
                        QueueOperation::PUSH);
    m_package_queue.emplace(pkg[PackageField::NAME], std::move(pkg));
}


void
Tab::pop_pkg(const std::string &name)
{
    if (m_package_queue.empty()) return;

    logger[Level::INFO, m_domain]("Removing package {} from the \"{}\" queue",
                                  name, m_tab_name);

    m_queue_signal.emit(m_tab_name, name, QueueOperation::POP);
    m_package_queue.erase(name);
}


auto
Tab::contains_pkg(const std::string &name) const -> bool
{
    return m_package_queue.contains(name);
}


auto
Tab::get_content_box() -> Gtk::Box *
{
    return m_content_box;
}


void
Tab::toggle_spinner()
{
    logger[Level::TRACE, m_domain]("{} spinner",
                                   m_spinner_running ? "Hiding" : "Showing");

    if (m_spinner_running)
    {
        m_content_box->remove(*m_spinner);
        m_content_box->set_valign(Gtk::ALIGN_START);
        m_spinner->stop();
    }
    else
    {
        m_content_box->set_valign(Gtk::ALIGN_FILL);
        m_content_box->foreach([this](Gtk::Widget &child) -> void
                               { m_content_box->remove(child); });
        m_content_box->pack_start(*m_spinner);
        m_spinner->start();
    }

    m_spinner_running = !m_spinner_running;
}
