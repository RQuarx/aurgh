#pragma once
#include <gtkmm/builder.h>
#include <gtkmm/window.h>

#include "client.hh"
#include "widgets/searchbar.hh"


namespace aurgh
{
    class window final : public Gtk::Window
    {
    public:
        window();

    private:
        widget::searchbar       m_searchbar;
        std::unique_ptr<client> m_client;
    };
}
