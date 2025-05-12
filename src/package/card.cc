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
#include <gtkmm/label.h>
#include <gtkmm/box.h>
#include <json/value.h>

#include <utility>

#include "package/card.hh"
#include "logger.hh"
#include "utils.hh"

using pkg::Card;


Card::Card(
    Json::Value package,
    const std::string &ui_file,
    const std::vector<str_pair> &installed_aur_packages,
    const std::shared_ptr<Logger> &logger,
    std::shared_ptr<Actions> &actions,
    int32_t spacing
) :
    m_installed_package(installed_aur_packages),
    m_actions(actions),
    m_logger(logger),
    m_package(std::move(package)),
    m_default_spacing(spacing),
    m_button_dimmed(false)
{
    auto builder = Gtk::Builder::create_from_file(ui_file);

    builder->get_widget("card_box", m_card);
    builder->get_widget("action_button", m_action_button);
    builder->get_widget("version_label", m_version_label);
    builder->get_widget("description_label", m_desc_label);
    builder->get_widget("name_link", m_name_link);
    builder->get_widget("name_label", m_name_label);
    builder->get_widget("popularity_label", m_popularity_label);
    builder->get_widget("votes_label", m_votes_label);

    add(*m_card);
    set_valign(Gtk::ALIGN_START);
    set_halign(Gtk::ALIGN_FILL);
    set_vexpand(false);
    set_hexpand(true);
    GtkUtils::set_margin(*this, m_default_spacing);

    setup();

    show_all_children();
}


auto
Card::get_action_button() -> Gtk::Button*&
{ return m_action_button; }


auto
Card::setup() -> bool
{
    m_version_label->set_label(m_package["Version"].asString());
    m_desc_label->set_label(m_package["Description"].asString());

    std::string url    = m_package["URL"].asString();
    std::string markup = "<b>{}</b>";

    if (m_package["OutOfDate"] != Json::Value::null) {
        markup = utils::format(markup, "<span foreground=\"red\">{}</span>");
    }

    m_name_label->set_markup(utils::format(markup, m_package["Name"].asString()));

    m_name_link->set_tooltip_text(url);
    m_name_link->set_uri(url);

    m_name_link->signal_clicked().connect([this](){
        m_name_link->set_visited();
    });

    markup = "<sub>{}</sub>";

    uint32_t votes   = m_package["NumVotes"].asUInt();
    float popularity =
        std::roundf(m_package["Popularity"].asFloat() * 100) / 100;

    m_popularity_label->set_markup(utils::format(markup, popularity));
    m_votes_label->set_markup(utils::format(markup, votes));

    std::string pkg_name    = m_package["Name"].asString();
    std::string pkg_version = m_package["Version"].asString();

    std::string icon_name;
    str_pair    pkg(pkg_name, pkg_version);
    int8_t      result = find_package(pkg);

    if      (result == -1) { icon_name = "document-save-symbolic"; }
    else if (result == 0)  { icon_name = "edit-delete-symbolic"; }
    else                   { icon_name = "view-refresh-symbolic"; }

    m_action_button->set_image_from_icon_name(icon_name);

    m_action_button->signal_clicked().connect(sigc::bind(
        sigc::mem_fun(*this, &Card::on_button_clicked),
        pkg::Type(result), pkg_name
    ));

    return true;
}


void
Card::on_button_clicked(pkg::Type result, const std::string &pkg_name)
{
    auto vec = m_actions->at(result);

    if (m_button_dimmed) {
        auto it = std::ranges::find(*vec, pkg_name);

        if (it != vec->end()) {
            vec->erase(it);
            m_action_button->set_opacity(1);
            m_button_dimmed = false;
        }
    } else {
        vec->push_back(pkg_name);
        m_action_button->set_opacity(0.5);
        m_button_dimmed = true;
    }
}


auto
Card::find_package(const str_pair &package) const -> int8_t
{
    if (utils::find(m_installed_package, package)) return 0;
    for (const auto &[pkg_name, _] : m_installed_package) {
        if (pkg_name == package.first) return 1;
    }
    return -1;
}