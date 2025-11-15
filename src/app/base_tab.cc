#include <gtkmm.h>

#include "app/base_tab.hh"

using app::BaseTab;
using app::CriteriaWidgets;


auto
CriteriaWidgets::get_values() const
    -> std::tuple<std::string, std::string, std::string, bool>
{
    return std::make_tuple(search_by->get_active_id(), sort_by->get_active_id(),
                           search_entry->get_text(), reverse->get_active());
}


BaseTab::BaseTab(BaseObjectType                   *p_object,
                 const Glib::RefPtr<Gtk::Builder> &p_builder)
    : Gtk::Box(p_object)
{
    p_builder->get_widget("content", m_content_box);
}


auto
BaseTab::set_name(std::string p_tab_name) -> BaseTab &
{
    m_tab_name = std::move(p_tab_name);
    return *this;
}


auto
BaseTab::signal_queue_update() -> signal_signature_queue
{
    return m_queue_signal;
}


auto
BaseTab::get_name() const -> std::string
{
    return m_tab_name;
}


void
BaseTab::push_pkg(Package &&p_pkg)
{
    m_package_queue.push(std::move(p_pkg));
    m_queue_signal.emit(m_package_queue);
}


void
BaseTab::pop_pkg()
{
    m_package_queue.pop();
    m_queue_signal.emit(m_package_queue);
}


auto
BaseTab::get_content_box() -> Gtk::Box *
{
    return m_content_box;
}
