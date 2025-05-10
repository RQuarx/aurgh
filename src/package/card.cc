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
#include <gtkmm/separator.h>
#include <gtkmm/button.h>
#include <gtkmm/image.h>
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
    const std::vector<str_pair> &installed_aur_packages,
    std::shared_ptr<Actions> &actions,
    const std::shared_ptr<Logger> &logger,
    int32_t spacing
) :
    m_action_button(Gtk::make_managed<Gtk::Button>()),
    m_installed_package(installed_aur_packages),
    m_actions(actions),
    m_logger(logger),
    m_package(std::move(package)),
    m_default_spacing(spacing)
{
    auto *card = Gtk::make_managed<Gtk::Box>();
    auto *info = Gtk::make_managed<Gtk::Box>();

    create_info_box(*info);
    create_action_button();

    card->set_spacing(m_default_spacing);
    card->pack_start(*info, true, true);
    card->pack_end(*m_action_button, true, true);
    GtkUtils::set_margin(*card, m_default_spacing);

    add(*card);
    set_shadow_type(Gtk::SHADOW_ETCHED_IN);
    set_valign(Gtk::ALIGN_START);
    set_halign(Gtk::ALIGN_FILL);
    set_vexpand(false);
    set_hexpand(true);
    GtkUtils::set_margin(*this, m_default_spacing);
}


auto
Card::get_action_button() -> Gtk::Button*
{ return m_action_button; }


void
Card::create_info_box(Gtk::Box &box) const
{
    auto *summary_info = Gtk::make_managed<Gtk::Box>();

    auto *name         = Gtk::make_managed<Gtk::Box>();
    auto *popularity = Gtk::make_managed<Gtk::Frame>();
    auto *version    = Gtk::make_managed<Gtk::Label>();
    auto *desc       = Gtk::make_managed<Gtk::Label>();

    std::string markup = "<b>{}</b>";
    if (m_package["OutOfDate"] != Json::Value::null) {
        markup = Utils::format(markup, "<span foreground=\"red\">{}</span>");
    }

    create_package_name(*name);
    create_popularity_frame(*popularity);
    version->set_text(m_package["Version"].asString());

    summary_info->pack_start(*name);
    summary_info->pack_start(*version);
    summary_info->pack_start(*popularity);
    summary_info->set_spacing(m_default_spacing);
    summary_info->set_halign(Gtk::ALIGN_START);
    summary_info->set_valign(Gtk::ALIGN_START);

    desc->set_label(m_package["Description"].asString());
    desc->set_halign(Gtk::ALIGN_START);
    desc->set_line_wrap(true);

    box.pack_start(*summary_info, true, true);
    box.pack_start(*desc, true, true);
    box.set_orientation(Gtk::ORIENTATION_VERTICAL);
    box.set_margin_left(m_default_spacing);
}


void
Card::create_package_name(Gtk::Box &box) const
{
    auto *img        = Gtk::make_managed<Gtk::Image>();
    auto *name_link           = Gtk::make_managed<Gtk::LinkButton>();
    auto *name_label = Gtk::make_managed<Gtk::Label>();

    std::string url    = m_package["URL"].asString();
    std::string markup = "<b>{}</b>";

    if (m_package["OutOfDate"] != Json::Value::null) {
        markup = Utils::format(markup, "<span foreground=\"red\">{}</span>");
    }

    name_label->set_markup(Utils::format(markup, m_package["Name"].asString()));

    name_link->add(*name_label);
    name_link->set_margin_left(m_default_spacing);
    name_link->set_tooltip_text(url);
    name_link->set_uri(url);

    name_link->signal_clicked().connect([name_link](){
        name_link->set_visited();
    });

    img->set_from_icon_name("package-x-generic-symbolic", Gtk::ICON_SIZE_MENU);

    box.pack_start(*img);
    box.pack_start(*name_link);
}


void
Card::create_popularity_frame(Gtk::Frame &frame) const
{
    const std::string_view markup = "<sub>{}</sub>";

    uint32_t votes   = m_package["NumVotes"].asUInt();
    float popularity =
        std::roundf(m_package["Popularity"].asFloat() * 100) / 100;

    auto *box                = Gtk::make_managed<Gtk::Box>();
    auto *votes_label      = Gtk::make_managed<Gtk::Label>();
    auto *sep                       = Gtk::make_managed<Gtk::Separator>();
    auto *popularity_label = Gtk::make_managed<Gtk::Label>();

    popularity_label->set_markup(Utils::format(markup, popularity));
    popularity_label->set_tooltip_text("Popularity");

    sep->set_orientation(Gtk::ORIENTATION_VERTICAL);

    votes_label->set_markup(Utils::format(markup, votes));
    votes_label->set_tooltip_text("Number of votes");

    box->pack_start(*popularity_label);
    box->pack_start(*sep);
    box->pack_start(*votes_label);
    box->set_spacing(m_default_spacing);
    GtkUtils::set_margin(*box, 0, m_default_spacing);

    frame.add(*box);
    frame.set_margin_left(m_default_spacing);
}


void
Card::create_action_button()
{
    std::string pkg_name    = m_package["Name"].asString();
    std::string pkg_version = m_package["Version"].asString();

    std::string icon_name;
    str_pair    pkg(pkg_name, pkg_version);
    int8_t      result = find_package(pkg);

    if      (result == -1) { icon_name = "document-save-symbolic"; }
    else if (result == 0)  { icon_name = "edit-delete-symbolic"; }
    else                   { icon_name = "view-refresh-symbolic"; }

    m_action_button->signal_clicked().connect(
    [result, this, pkg_name]() -> void
    {
        auto *vec = m_actions->at(pkg::Type(result));

        if (m_action_button->get_opacity() < 1) {
            auto it = std::ranges::find(*vec, pkg_name);

            if (it != vec->end()) {
                vec->erase(it);
                m_action_button->set_opacity(1);
            }

        } else {
            vec->push_back(pkg_name);
            m_action_button->set_opacity(
                m_action_button->get_opacity() / 2
            );
        }

#ifdef DEBUG
        m_logger->log(Logger::Debug, "\n{}", *m_actions);
#endif /* DEBUG */
    });

    m_action_button->set_image_from_icon_name(icon_name);
    m_action_button->set_valign(Gtk::ALIGN_CENTER);
    m_action_button->set_halign(Gtk::ALIGN_END);
    GtkUtils::set_margin(*m_action_button, m_default_spacing);
}


auto
Card::find_package(const str_pair &package) const -> int8_t
{
    std::vector<std::string> version;
    std::vector<std::string> name;

    version.reserve(m_installed_package.size());
    name.reserve(m_installed_package.size());

    if (Utils::find(m_installed_package, package)) return 0;
    if (Utils::find(name, package.first)) return 1;
    return -1;
}