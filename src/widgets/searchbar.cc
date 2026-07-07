#include <gtkmm.h>

#include "widgets/searchbar.hh"

using aurgh::widget::searchbar;


auto
searchbar::build(const Glib::RefPtr<Gtk::Builder> &builder) noexcept -> result<void>
try
{
    m_searchbar   = builder->get_widget<Gtk::SearchBar>("searchbar");
    m_searchentry = builder->get_widget<Gtk::SearchEntry>("searchentry");
    m_searchbar->connect_entry(*m_searchentry);

    return {};
}
catch (const Glib::Error &e)
{
    return error { "GLib error [{}]: {}", e.code(), e.what() }.unexpected();
}
catch (const std::exception &e)
{
    return error { "failed to build searchbar widgets: {}", e.what() }.unexpected();
}
