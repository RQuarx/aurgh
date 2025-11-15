#pragma once
#include <future>
#include <string>
#include <vector>

#include <glibmm/dispatcher.h>

namespace Gtk
{
    class Window;
    class Widget;
}


namespace app
{
    /* Provides a way to ask for user response/confirmation. */
    class ChoiceDialog
    {
    public:
        ChoiceDialog(Gtk::Window *p_parent);


        /* Sets the message that will be shown on the dialog. */
        auto set_message(std::string p_message) -> ChoiceDialog &;


        /**
         * Add a response that the user will be able to pick.
         *
         * [note]--------------------------------------------
         *
         * The stored responses are cleared once `ChoiceDialog::show_dialog`
         * or `ChoiceDialog::show_dialog_async` is called.
        */
        auto add_response(std::string p_response) -> ChoiceDialog &;


        /* Clear the responses vector. */
        auto clear_responses() -> ChoiceDialog &;


        /* Shows the dialog window to the user. */
        auto show_dialog() -> std::string;


        /* Shows the dialog window to the user, asynchronously. */
        auto show_dialog_async() -> std::future<std::string>;


        /**
         * Wrapper function for `ChoiceDialog`.
         *
         * [params]----------------------------
         *
         * `p_widget`:
         *   The provided widget's toplevel widget will be used
         *   as the parent window.
         *
         * `p_async`:
         *   If `true`, the function will use `ChoiceDialog::show_dialog_async`
         *   instead of `ChoiceDialog::show_dialog`.
         */
        static auto show_error(Gtk::Widget             *p_widget,
                               std::string              p_message,
                               std::vector<std::string> p_responses,
                               bool p_async = false) -> std::string;

    private:
        Gtk::Window *parent;

        std::string              message;
        std::vector<std::string> responses;
    };
}
