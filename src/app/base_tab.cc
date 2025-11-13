#include <gtkmm.h>

#include "app/base_tab.hh"

using app::BaseTab;
using app::CriteriaWidgets;

auto
CriteriaWidgets::get_string(this CriteriaWidgets &self)
    -> std::tuple<std::string, std::string, std::string, bool>
{
    return std::make_tuple(
        self.search_by->get_active_id(), self.sort_by->get_active_id(),
        self.search_entry->get_text(), self.reverse->get_active());
}


BaseTab::BaseTab(BaseObjectType                   *p_object,
                 const Glib::RefPtr<Gtk::Builder> &p_builder)
    : Gtk::Box(p_object)
{
    p_builder->get_widget("content", this->content_box);
}


auto
BaseTab::set_name(this BaseTab &self, std::string &&p_tab_name) -> BaseTab &
{
    self.tab_name = std::move(p_tab_name);
    return self;
}


auto
BaseTab::signal_queue_update() -> signal_signature_queue
{
    return this->queue_signal;
}


auto
BaseTab::get_name(this const BaseTab &self) -> std::string
{
    return self.tab_name;
}


void
BaseTab::push_pkg(Package &&p_pkg)
{
    this->package_queue.push(std::move(p_pkg));
    this->queue_signal.emit(this->package_queue);
}


void
BaseTab::pop_pkg()
{
    this->package_queue.pop();
    this->queue_signal.emit(this->package_queue);
}


auto
BaseTab::get_content_box(this BaseTab &self) -> Gtk::Box *
{
    return self.content_box;
}
