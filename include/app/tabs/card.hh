#pragma once
#include <memory>

#include <glibmm/signalproxy.h>
#include <sigc++/signal.h>
#include <json/value.h>

#include "app/package.hh"
#include "app/types.hh"

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
        enum Type
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
        auto get_widget( void ) -> Gtk::Box *;

        [[nodiscard]]
        auto is_valid( void ) -> bool;

        [[nodiscard]]
        auto get_package( void ) -> Package &;


        auto signal_on_add_to_queue( void ) -> Glib::SignalProxy<void>;
        auto signal_on_install( void ) -> Glib::SignalProxy<void>;
        auto signal_on_uninstall( void ) -> Glib::SignalProxy<void>;


    private:
        std::shared_ptr<Logger> m_logger;

        Package m_pkg;

        Gtk::Box *m_card;

        Gtk::ToggleButton *m_install;
        Gtk::ToggleButton *m_add_to_queue;
        Gtk::ToggleButton *m_uninstall;
    };
}