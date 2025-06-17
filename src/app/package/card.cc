/**
 * aurgh Copyright (C) 2025 RQuarx
 *
 * This file is part of aurgh
 *
 * aurgh is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * aurgh is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with aurgh. If not, see <https://www.gnu.org/licenses/>.
 */

#include <gtkmm/linkbutton.h>
#include <gtkmm/builder.h>
#include <gtkmm/button.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>
#include <gtkmm/box.h>

#include <utility>

#include "package/client.hh"
#include "package/card.hh"
#include "utils.hh"
#include "data.hh"

using pkg::Card;


Card::Card(BaseObjectType      *p_cobject,
           const builder_t     &p_b,
           json                 p_pkg) :
    Gtk::Frame(p_cobject),
    m_package(std::move(p_pkg)),
    m_type(pkg::Install),
    m_button_dimmed(false)
{
#if GTK4
    m_card             = p_b->get_widget<Gtk::Box>("card_box");
    m_action_button    = p_b->get_widget<Gtk::Button>("action_button");
    m_version_label    = p_b->get_widget<Gtk::Label>("version_label");
    m_desc_label       = p_b->get_widget<Gtk::Label>("description_label");
    m_name_link        = p_b->get_widget<Gtk::LinkButton>("name_link");
    m_name_label       = p_b->get_widget<Gtk::Label>("name_label");
    m_outofdate_label  = p_b->get_widget<Gtk::Label>("outofdate_label");
    m_popularity_label = p_b->get_widget<Gtk::Label>("popularity_label");
    m_votes_label      = p_b->get_widget<Gtk::Label>("votes_label");
    m_package_icon     = p_b->get_widget<Gtk::Image>("package_image");
#else
    p_b->get_widget("card_box", m_card);
    p_b->get_widget("action_button", m_action_button);
    p_b->get_widget("version_label", m_version_label);
    p_b->get_widget("description_label", m_desc_label);
    p_b->get_widget("name_link", m_name_link);
    p_b->get_widget("name_label", m_name_label);
    p_b->get_widget("outofdate_label", m_outofdate_label);
    p_b->get_widget("popularity_label", m_popularity_label);
    p_b->get_widget("votes_label", m_votes_label);
    p_b->get_widget("package_image", m_package_icon);
#endif

    setup();
}


auto
Card::signal_action_pressed() -> action_signal
{ return m_signal; }


auto
Card::setup() -> bool
{
    str
        pkg_desc    = m_package["Description"].asString(),
        pkg_version = m_package["Version"    ].asString(),
        pkg_name    = m_package["Name"       ].asString(),
        url         = m_package["URL"        ].asString(),
        markup      = "<big>{}</big>",
        icon_name;

    m_desc_label->set_label(pkg_desc);

    if (m_package["OutOfDate"] != json::null) {
        m_outofdate_label->set_label("(Out-of-date)");
    }

    m_name_label->set_markup(utils::format(markup, pkg_name));
    m_name_link ->set_tooltip_text(url);
    m_name_link ->set_uri(url);

    markup = "<b>{}</b>";

    uint32_t votes      = m_package["NumVotes"].asUInt();
    float    popularity =
        std::roundf(m_package["Popularity"].asFloat() * 100) / 100;

    m_version_label   ->set_markup(utils::format(markup, pkg_version));
    m_popularity_label->set_markup(utils::format(markup, popularity ));
    m_votes_label     ->set_markup(utils::format(markup, votes      ));

    str_pair pkg = { pkg_name, pkg_version };
    m_type       = pkg::Type(find_package(pkg));

    switch (m_type) {
    case pkg::Install: icon_name = "document-save-symbolic"; break;
    case pkg::Remove:  icon_name = "edit-delete-symbolic";   break;
    default:           icon_name = "view-refresh-symbolic";  break;
    }

    m_action_button->set_image_from_icon_name(icon_name);
    m_action_button->signal_clicked().connect(sigc::mem_fun(
        *this, &Card::on_button_clicked
    ));

    try {
        m_package_icon->set(data::package_icon_file);
#if GTK4
        m_package_icon->set_icon_size(Gtk::IconSize::LARGE);
#else
        m_package_icon->set_pixel_size(Gtk::ICON_SIZE_LARGE_TOOLBAR);
#endif
    } catch (const Glib::Error &e) {
        data::logger->log(
            Logger::Warn,
            "Icon path not found, using default icon name "
            "\"package-x-generic-symbolic\""
        );
#if GTK4
        m_package_icon->set_from_icon_name("package-x-generic-symbolic");
        m_package_icon->set_icon_size(Gtk::IconSize::LARGE);
#else
        m_package_icon->set_from_icon_name("package-x-generic-symbolic",
                                           Gtk::ICON_SIZE_LARGE_TOOLBAR);
#endif
    }

    return true;
}


void
Card::on_button_clicked()
{
    if (m_button_dimmed) {
        m_action_button->set_opacity(ACTIVE_OPACITY);
        m_button_dimmed = false;

        m_signal.emit(m_type, false, m_package);
    } else {
        m_action_button->set_opacity(INACTIVE_OPACITY);
        m_button_dimmed = true;

        m_signal.emit(m_type, true, m_package);
    }
}


auto
Card::find_package(const str_pair &p_pkg) -> int8_t
{
    if (data::pkg_client->find_pkg(p_pkg.first) != nullptr) {
        return 0;
    }
    return -1;
    // if (utils::find(*m_installed_pkgs, package)) return 0;
    // for (const auto &[pkg_name, _] : *m_installed_pkgs) {
        // if (pkg_name == package.first) return 1;
    // }
    // return -1;
}