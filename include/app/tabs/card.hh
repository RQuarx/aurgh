#pragma once
#include <glibmm/signalproxy.h>

#include "package.hh"

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
        enum class Type : std::uint8_t
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
        Package pkg;

        Gtk::Frame *card;

        Gtk::ToggleButton *install;
        Gtk::ToggleButton *add_to_queue;
        Gtk::ToggleButton *uninstall;
    };
}
