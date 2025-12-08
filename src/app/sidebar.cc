#include <gtkmm.h>

#include "app/sidebar.hh"
#include "logger.hh"

using app::Sidebar;


Sidebar::Sidebar(BaseObjectType                   *p_object,
                 const Glib::RefPtr<Gtk::Builder> &p_builder,
                 Gtk::ToggleButton                *p_sidebar_toggle)
    : Gtk::Box(p_object), m_sidebar_toggle(p_sidebar_toggle),
      m_toggle_icon(dynamic_cast<Gtk::Image *>(m_sidebar_toggle->get_child()))
{
    m_sidebar_toggle->signal_clicked().connect(
        sigc::mem_fun(*this, &Sidebar::on_button_toggle));

    p_builder->get_widget("aurgh_sidebar_install_button", m_install_button);
    p_builder->get_widget("aurgh_sidebar_container", m_container);
}


void
Sidebar::add_queue_signal(Tab::signal_signature_queue p_signal)
{
    p_signal.connect(sigc::mem_fun(*this, &Sidebar::on_queue_mutation));
}


void
Sidebar::on_button_toggle()
{
    const bool  activated { m_sidebar_toggle->get_active() };
    const char *icon_name { activated ? "pan-start-symbolic"
                                      : "pan-end-symbolic" };

    logger[Level::TRACE, "app::sidebar"]("Sidebar is {}",
                                         activated ? "shown" : "hidden");

    m_toggle_icon->set_from_icon_name(icon_name, Gtk::ICON_SIZE_BUTTON);
    this->set_visible(activated);
}


void
Sidebar::on_queue_mutation(const std::string              &p_tab_name,
                           std::map<std::string, Package> &p_package_queue)
{
    if (!m_queue_boxes.contains(p_tab_name))
    {
        logger[Level::DEBUG, "app::sidebar"](
            "Adding a new queue box for tab {}", p_tab_name);

        auto *queue_box { Gtk::make_managed<Gtk::Box>(
            Gtk::ORIENTATION_VERTICAL) };

        queue_box->set_halign(Gtk::ALIGN_START);
        queue_box->set_valign(Gtk::ALIGN_START);
        queue_box->set_margin_start(10);

        auto *title { Gtk::make_managed<Gtk::Label>() };

        title->set_markup(std::format("<big><b>{}</b></big>", p_tab_name));
        title->set_margin_bottom(10);
        title->set_halign(Gtk::ALIGN_START);
        title->set_valign(Gtk::ALIGN_START);
        title->set_name("queue-title");

        queue_box->pack_start(*title);
        queue_box->show_all_children();

        m_container->pack_end(*queue_box);

        m_queue_boxes[p_tab_name] = queue_box;
    }

    Gtk::Box *box { m_queue_boxes[p_tab_name] };

    box->foreach(
        [box, p_tab_name](Gtk::Widget &p_child) -> void
        {
            Gtk::Label *label { dynamic_cast<Gtk::Label *>(&p_child) };
            if (label->get_name() != "queue-title") box->remove(*label);
        });

    if (p_package_queue.empty())
    {
        m_container->remove(*m_queue_boxes[p_tab_name]);
        m_queue_boxes.erase(p_tab_name);

        if (m_queue_boxes.empty())
        {
            m_sidebar_toggle->set_visible(false);
            set_visible(false);
        }

        return;
    }

    if (m_sidebar_toggle->get_active()) set_visible(true);
    box->set_visible(true);
    m_sidebar_toggle->set_visible(true);

    for (auto &[name, _] : p_package_queue)
    {
        auto *label { Gtk::make_managed<Gtk::Label>(name) };
        label->set_halign(Gtk::ALIGN_START);
        label->set_valign(Gtk::ALIGN_START);

        box->pack_start(*label);
    }

    box->show_all_children();
}
