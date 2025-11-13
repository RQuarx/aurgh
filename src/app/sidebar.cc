#include <gtkmm/box.h>
#include <gtkmm/image.h>
#include <gtkmm/togglebutton.h>

#include "app/sidebar.hh"
#include "log.hh"

using app::Sidebar;


Sidebar::Sidebar(Gtk::ToggleButton *p_sidebar_toggle, Gtk::Box *p_container)
    : sidebar_toggle(p_sidebar_toggle), container(p_container),
      toggle_icon(dynamic_cast<Gtk::Image *>(sidebar_toggle->get_child()))
{
    sidebar_toggle->signal_clicked().connect(
        sigc::mem_fun(*this, &Sidebar::on_button_toggle));
}


void
Sidebar::on_button_toggle()
{
    const bool  activated { this->sidebar_toggle->get_active() };
    const char *icon_name { activated ? "pan-start-symbolic"
                                      : "pan-end-symbolic" };

    logger.log<DEBUG>("Sidebar is {}", activated ? "shown" : "hidden");


    this->toggle_icon->set_from_icon_name(icon_name, Gtk::ICON_SIZE_BUTTON);
    this->container->set_visible(activated);
}
