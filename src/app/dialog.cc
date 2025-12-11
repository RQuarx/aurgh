#include <future>
#include <utility>

#include <gtkmm.h>

#include "app/dialog.hh"
#include "app/utils.hh"

using app::ChoiceDialog;


namespace
{
    [[nodiscard]]
    auto get_window(Gtk::Widget *obj) -> Gtk::Window *
    {
        return reinterpret_cast<Gtk::Window *>(obj->get_toplevel());
    }


    [[nodiscard]]
    auto
    show_error_impl(Gtk::Widget             *widget,
                    std::string              message,
                    std::vector<std::string> responses) -> ChoiceDialog
    {
        ChoiceDialog dialog { get_window(widget) };

        dialog.set_message(std::move(message));
        for (auto &response : responses)
            dialog.add_response(std::move(response));

        return dialog;
    }
}


ChoiceDialog::ChoiceDialog(Gtk::Window *parent) : parent(parent) {}


auto
ChoiceDialog::set_message(std::string message) -> ChoiceDialog &
{
    this->message = std::move(message);
    return *this;
}


auto
ChoiceDialog::add_response(std::string response) -> ChoiceDialog &
{
    this->responses.emplace_back(std::move(response));
    return *this;
}


auto
ChoiceDialog::clear_responses() -> ChoiceDialog &
{
    this->responses.clear();
    return *this;
}


auto
ChoiceDialog::show_dialog() -> std::string
{
    std::string response;

    Gtk::Dialog dialog { "Select an option", *parent, true };
    Gtk::Label  label { this->message };

    label.set_margin_top(10);
    label.set_margin_bottom(10);
    label.set_margin_start(10);
    label.set_margin_end(10);
    label.set_line_wrap(true);
    dialog.get_content_area()->pack_start(label, true, true);
    label.show();

    for (size_t i { 0 }; i < this->responses.size(); ++i)
        dialog.add_button(this->responses[i], static_cast<int>(i));

    dialog.set_visible();

    int id { dialog.run() };
    if (id >= 0 && static_cast<size_t>(id) < responses.size())
        response = responses[id];

    responses.clear();

    return response;
}


auto
ChoiceDialog::show_dialog_async() -> std::future<std::string>
{
    auto promise { std::make_shared<std::promise<std::string>>() };
    auto fut { promise->get_future() };

    Glib::MainContext::get_default()->invoke(
        [this, promise]() mutable -> bool
        {
            auto dialog { std::make_shared<Gtk::Dialog>("Select an option",
                                                        *parent, true) };
            auto label { std::make_shared<Gtk::Label>(message) };

            label->set_margin_top(10);
            label->set_margin_bottom(10);
            label->set_margin_start(10);
            label->set_margin_end(10);
            label->set_line_wrap(true);

            dialog->get_content_area()->pack_start(*label, true, true);
            label->show();

            for (size_t i = 0; i < responses.size(); ++i)
                dialog->add_button(responses[i], static_cast<int>(i));

            auto *raw { dialog.get() };
            dialog->signal_response().connect(
                [p = promise, dlg = std::move(dialog),
                 this](int id) mutable -> void
                {
                    std::string res;
                    if (id >= 0 && static_cast<size_t>(id) < responses.size())
                        res = responses[id];

                    responses.clear();
                    p->set_value(res);
                });
            raw->set_visible(true);
            return true;
        });

    return fut;
}


auto
ChoiceDialog::show_error(Gtk::Widget             *widget,
                         std::string              message,
                         std::vector<std::string> responses) -> std::string
{
    return show_error_impl(widget, std::move(message),
                           std::move(responses))
        .show_dialog();
}


auto
ChoiceDialog::show_error_async(Gtk::Widget             *widget,
                               std::string              message,
                               std::vector<std::string> responses)
    -> std::future<std::string>
{
    return show_error_impl(widget, std::move(message),
                           std::move(responses))
        .show_dialog_async();
}

