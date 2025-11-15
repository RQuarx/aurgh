#include <future>

#include <gtkmm.h>

#include "app/dialog.hh"

using app::ChoiceDialog;


ChoiceDialog::ChoiceDialog(Gtk::Window *p_parent) : parent(p_parent)
{
    // dispatcher.connect();
}


auto
ChoiceDialog::set_message(std::string &&p_message) -> ChoiceDialog &
{
    this->message = std::move(p_message);
    return *this;
}


auto
ChoiceDialog::add_response(std::string &&p_response) -> ChoiceDialog &
{
    this->responses.emplace_back(std::move(p_response));
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
