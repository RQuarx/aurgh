#include <gtkmm.h>

#include "app/tab.hh"
#include "log.hh"

using app::CriteriaWidgets;
using app::Tab;


auto
CriteriaWidgets::get_values() const
    -> std::tuple<std::string, std::string, std::string, bool>
{
    return std::make_tuple(search_by->get_active_id(), sort_by->get_active_id(),
                           search_entry->get_text(), reverse->get_active());
}


Tab::Tab(BaseObjectType *p_object, const Glib::RefPtr<Gtk::Builder> &p_builder)
    : Gtk::Box(p_object)
{
    p_builder->get_widget("content", m_content_box);
}


auto
Tab::set_name(std::string p_tab_name) -> Tab &
{
    m_tab_name = std::move(p_tab_name);
    return *this;
}


auto
Tab::signal_queue_update() -> signal_signature_queue
{
    return m_queue_signal;
}


auto
Tab::get_name() const -> std::string
{
    return m_tab_name;
}


void
Tab::push_pkg(Package p_pkg)
{
    logger.log<LogLevel::INFO>("Adding package {} to queue", p_pkg[PKG_NAME]);

    m_package_queue.emplace(p_pkg[PKG_NAME], std::move(p_pkg));
    m_queue_signal.emit(m_tab_name, m_package_queue);
}


void
Tab::pop_pkg(const std::string &p_name)
{
    if (m_package_queue.empty()) return;

    logger.log<LogLevel::INFO>("Removing package {} to queue", p_name);

    m_package_queue.erase(p_name);
    m_queue_signal.emit(m_tab_name, m_package_queue);
}


auto
Tab::contains_pkg(const std::string &p_name) const -> bool
{
    return m_package_queue.contains(p_name);
}


auto
Tab::get_content_box() -> Gtk::Box *
{
    return m_content_box;
}
