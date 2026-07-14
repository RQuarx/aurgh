#include <gtkmm.h>

#include "window.hh"

using aurgh::window;


window::window()
{
    auto builder = Gtk::Builder::create_from_resource("/org/kei/aurgh/window.ui");

    this->set_title("aurgh");
    this->set_child(*builder->get_widget<Gtk::Box>("main_container"));

    if (auto res = m_searchbar.build(builder); !res) { /* TODO: Handle error */ }
}
