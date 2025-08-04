#pragma once
#include <memory>
#include <sigc++/signal.h>
#include <json/value.h>
#include "app/package.hh"
#include "app/types.hh"

namespace Gtk
{
    class Button;
    class Box;
}
class Logger;


class Card
{
public:
    enum button_type
    {
        BUTTON_INSTALL,
        BUTTON_ADD_TO_QUEUE,
        BUTTON_UNINSTALL
    };
    using button_sig = sigc::signal<void_or_err, button_type>;

    Card( const std::shared_ptr<Logger> &logger,
          const std::string               &pkg_name );

    Card( const std::shared_ptr<Logger> &logger,
          const Json::Value               &pkg );

    ~Card( void );


    [[nodiscard]]
    auto on_button_clicked( void ) -> button_sig;

    [[nodiscard]]
    auto get_widget( void ) -> Gtk::Box *;

    [[nodiscard]]
    auto is_valid( void ) -> bool;


private:
    std::shared_ptr<Logger> m_logger;

    Package m_pkg;

    Gtk::Box *m_card;

    Gtk::Button *m_install;
    Gtk::Button *m_add_to_queue;
    Gtk::Button *m_uninstall;
};