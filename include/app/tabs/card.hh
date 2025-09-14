#pragma once
#include <memory>

#include <glibmm/signalproxy.h>
#include <sigc++/signal.h>
#include <json/value.h>

#include "app/package.hh"

namespace Gtk
{
    class ToggleButton;
    class Box;
}
class Logger;


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


        Card( const std::shared_ptr<Logger> &p_logger,
              const std::string             &p_pkg_name,
              const Type                    &p_card_type );

        Card( const std::shared_ptr<Logger> &p_logger,
              const Json::Value             &p_pkg,
              const Type                    &p_card_type );

        ~Card( void );


        [[nodiscard]]
        auto get_widget() -> Gtk::Box *;

        [[nodiscard]]
        auto is_valid() -> bool;

        [[nodiscard]]
        auto get_package() -> Package &;


        auto signal_on_add_to_queue() -> Glib::SignalProxy<void>;
        auto signal_on_install() -> Glib::SignalProxy<void>;
        auto signal_on_uninstall() -> Glib::SignalProxy<void>;


    private:
        std::shared_ptr<Logger> m_logger;

        Package m_pkg;

        Gtk::Box *m_card;

        Gtk::ToggleButton *m_install;
        Gtk::ToggleButton *m_add_to_queue;
        Gtk::ToggleButton *m_uninstall;
    };
}