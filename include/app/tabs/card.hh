#pragma once
#include <glibmm/signalproxy.h>
#include <json/value.h>
#include <sigc++/signal.h>

#include "app/package.hh"

namespace Gtk
{
    class ToggleButton;
    class Frame;
    class Box;
}


namespace app
{
    class Card
    {
    public:
        enum Type : uint8_t
        {
            INSTALL,
            UNINSTALL
        };


        Card(const std::string &p_pkg_name, const Type &p_card_type);

        Card(const Json::Value &p_pkg, const Type &p_card_type);

        ~Card();


        [[nodiscard]]
        auto get_widget() -> Gtk::Frame *;

        [[nodiscard]]
        auto is_valid() -> bool;

        [[nodiscard]]
        auto get_package() -> Package &;


        auto signal_on_add_to_queue() -> Glib::SignalProxy<void>;
        auto signal_on_install() -> Glib::SignalProxy<void>;
        auto signal_on_uninstall() -> Glib::SignalProxy<void>;


    private:
        Package m_pkg;

        Gtk::Frame *m_card;

        Gtk::ToggleButton *m_install;
        Gtk::ToggleButton *m_add_to_queue;
        Gtk::ToggleButton *m_uninstall;
    };
}
