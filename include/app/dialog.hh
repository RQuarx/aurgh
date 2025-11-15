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
    class ChoiceDialog
    {
    public:
        ChoiceDialog(Gtk::Window *p_parent);


        auto set_message(std::string &&p_message) -> ChoiceDialog &;


        auto add_response(std::string &&p_response) -> ChoiceDialog &;


        auto clear_responses() -> ChoiceDialog &;


        auto show_dialog() -> std::string;


        auto show_dialog_async() -> std::future<std::string>;


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
