#include <gtkmm/builder.h>
#include <gtkmm/button.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>

#include "dialog.hh"
#include "utils.hh"
#include "data.hh"


Dialog::Dialog(BaseObjectType  *cobject,
               const builder_t &b,
               const str       &p_title,
               const str       &p_message,
               bool             use_dark) :
    Gtk::Window(cobject)
{
    data::logger->log(
        Logger::Debug,
        R"(Creating a dialog window with a title "{}" and message "{}".)",
        p_title, p_message
    );

    Gtk::Image *icon = nullptr;
    Gtk::Label
        *message = nullptr,
        *title   = nullptr;

#if GTK4
    title   = b->get_widget<Gtk::Label>("confirmation_title");
    message = b->get_widget<Gtk::Label>("confirmation_message");
    icon    = b->get_widget<Gtk::Image>("confirmation_title_img");

    m_accept_button  = b->get_widget<Gtk::Button>("confirmation_accept");
    m_decline_button = b->get_widget<Gtk::Button>("confirmation_decline");

    icon->set_icon_size(Gtk::IconSize::LARGE);
#else
    b->get_widget("confirmation_title",     title);
    b->get_widget("confirmation_message",   message);
    b->get_widget("confirmation_title_img", icon);

    b->get_widget("confirmation_accept",  m_accept_button);
    b->get_widget("confirmation_decline", m_decline_button);

    icon->set_pixel_size(Gtk::ICON_SIZE_LARGE_TOOLBAR);
#endif

    icon->set(utils::get_ui_file(
              std::format("question_{}.svg", use_dark ? "dark" : "light"),
              (*data::config->get_config())["paths"]["ui-path"].asString()));

    title  ->set_markup(std::format("<big>{}</big>", p_title));
    message->set_label(p_message);

#if GTK4
    set_transient_for(*data::app->get_run_window());
#else
    set_transient_for(*data::app->get_active_window());
    show_all_children();
#endif
    set_application(data::app);
}


auto
Dialog::wait_for_response() -> bool
{
    m_loop = Glib::MainLoop::create();

    m_accept_button->signal_clicked().connect([this]() {
        m_response = true;
        if (m_loop && m_loop->is_running()) m_loop->quit();
        set_visible(false);
    });

    m_decline_button->signal_clicked().connect([this]() {
        m_response = false;
        if (m_loop && m_loop->is_running()) m_loop->quit();
        set_visible(false);
    });

    set_visible();
    m_loop->run();

    return m_response;
}