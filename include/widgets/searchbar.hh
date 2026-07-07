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
        auto build(const Glib::RefPtr<Gtk::Builder> &builder) noexcept -> result<void>;

    private:
        Gtk::SearchBar *m_searchbar;

        Gtk::SearchEntry *m_searchentry;
    };
}
