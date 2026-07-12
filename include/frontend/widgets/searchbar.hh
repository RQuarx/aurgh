#pragma once
#include <gtkmm/builder.h>

#include "widgets/base.hh"

namespace Gtk
{
    class SearchBar;
    class SearchEntry;
}


namespace aurgh::widget
{
    class searchbar final : public base
    {
    public:
        [[nodiscard]]
        auto build(const Glib::RefPtr<Gtk::Builder> &builder) noexcept -> result<void>;


        [[nodiscard]]
        auto signal_on_search() -> sigc::signal<void(Glib::ustring)>;

    private:
        Gtk::SearchBar   *m_searchbar;
        Gtk::SearchEntry *m_searchentry;

        sigc::signal<void(Glib::ustring)> m_signal_on_search;
    };
}
