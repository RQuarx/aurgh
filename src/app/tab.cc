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
    : Gtk::Box(object), m_domain(std::move(domain)),
      m_tab_name(std::move(tab_name))
{
    builder->get_widget("content", m_content_box);
}


auto
Tab::set_tab_name(std::string tab_name) -> Tab &
{
    m_tab_name = std::move(tab_name);
    return *this;
}


auto
Tab::set_domain_name(std::string domain) -> Tab &
{
    m_domain = std::move(domain);
    return *this;
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
Tab::push_pkg(Package pkg)
{
    logger[Level::INFO, m_domain]("Adding package {} to the \"{}\" queue",
                                  pkg[PKG_NAME], m_tab_name);

    m_queue_signal.emit(m_tab_name, pkg[PKG_NAME], QueueOperation::PUSH);
    m_package_queue.emplace(pkg[PKG_NAME], std::move(pkg));
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
