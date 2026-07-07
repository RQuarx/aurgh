#pragma once
#include <gtkmm/window.h>
#include <gtkmm/builder.h>

#include "widgets/searchbar.hh"


namespace aurgh
{
    class window final : public Gtk::Window
    {
    public:
        window();

    private:
        widget::searchbar m_searchbar;
    };
}