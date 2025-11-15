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
    /**
     * Provides a way to represent packages in the UI in form of cards.
     *
     * [note]----------------------------------------------------------
     *
     * Each Card displays information about a single package and provides
     * buttons for installing, uninstalling, or adding it to a queue.
     */
    class Card
    {
    public:
        /* The type of the card, which affects which buttons are visible. */
        enum class Type : std::uint8_t
        {
            INSTALL,  /* `Card` is for installing, shows the install button. */
            UNINSTALL /* `Card` is for uninsalling,
                          shows the uninstall button. */
        };


        /**
         * Construct a `Card` for a given package.
         *
         * [params]-------------------------------
         *
         * `p_pkg`:
         *   A JSON representation of a package, see `Package` for information
         *   regarding the package's JSON format.
         */
        Card(const Json::Value &p_pkg, const Type &p_card_type);


        /* Get the card's widget. */
        [[nodiscard]]
        auto get_widget() -> Gtk::Frame *;


        /* Checks whether the provided JSON at construction is valid. */
        [[nodiscard]]
        auto is_valid() -> bool;


        /* Get a reference to the underlying `Package` object. */
        [[nodiscard]]
        auto get_package() -> Package &;


        /**
         * Get the signal emitted when the "Add to Queue" button is clicked.
         *
         * [note]-------------------------------------------------------------
         *
         * Connect to this signal to perform actions when the user wants to
         * queue the package for install/uninstall.
         */
        auto signal_on_add_to_queue() -> Glib::SignalProxy<void>;


        /* Get the signal emitted when the "Install" button is clicked. */
        auto signal_on_install() -> Glib::SignalProxy<void>;


        /* Get the signal emitted when the "Uninstall" button is clicked. */
        auto signal_on_uninstall() -> Glib::SignalProxy<void>;

    private:
        Package m_pkg;

        Gtk::Frame        *m_card;
        Gtk::ToggleButton *m_install;
        Gtk::ToggleButton *m_add_to_queue;
        Gtk::ToggleButton *m_uninstall;
    };
}
