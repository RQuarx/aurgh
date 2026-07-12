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

    m_searchentry->signal_search_changed().connect(
        [this]() { m_signal_on_search.emit(m_searchentry->get_text()); });

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


auto
searchbar::signal_on_search() -> sigc::signal<void(Glib::ustring)>
{ return m_signal_on_search; }
