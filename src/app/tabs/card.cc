#include <gtkmm.h>

#include "app/tabs/card.hh"

using app::Card;


Card::Card(const Json::Value &p_pkg, const Type &p_card_type)
    : m_pkg(p_pkg), m_card(Gtk::make_managed<Gtk::Frame>()),
      m_install(Gtk::make_managed<Gtk::ToggleButton>()),
      m_add_to_queue(Gtk::make_managed<Gtk::ToggleButton>()),
      m_uninstall(Gtk::make_managed<Gtk::ToggleButton>())
{
    auto *main_box { Gtk::make_managed<Gtk::Box>() };

    auto *info_box { Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL) };
    auto *button_box { Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL) };

    auto *name_ver { Gtk::make_managed<Gtk::Label>() };
    auto *desc { Gtk::make_managed<Gtk::Label>() };
    auto *keywords { Gtk::make_managed<Gtk::Box>() };

    name_ver->set_markup(
        std::format("<span size='xx-large'><b>{}</b></span>    {}",
                    m_pkg[PKG_NAME], m_pkg[PKG_VERSION]));
    name_ver->set_halign(Gtk::ALIGN_START);

    desc->set_markup(std::format("<big>{}</big>", m_pkg[PKG_DESC]));
    desc->set_halign(Gtk::ALIGN_START);
    desc->set_valign(Gtk::ALIGN_START);
    desc->set_line_wrap();

    keywords->set_spacing(10);
    keywords->set_halign(Gtk::ALIGN_START);

    for (const std::string &kw : m_pkg.get_keywords())
    {
        auto *frame { Gtk::make_managed<Gtk::Frame>() };
        auto *label { Gtk::make_managed<Gtk::Label>(kw) };

        label->set_margin_bottom(10);
        label->set_margin_start(10);
        label->set_margin_top(10);
        label->set_margin_end(10);

        frame->set_hexpand(false);
        frame->set_halign(Gtk::ALIGN_START);
        frame->add(*label);
        keywords->pack_start(*frame);
    }

    info_box->set_margin_bottom(10);
    info_box->set_margin_start(10);
    info_box->set_margin_top(10);
    info_box->set_margin_end(10);
    info_box->set_spacing(10);
    info_box->pack_start(*name_ver);
    info_box->pack_start(*desc);
    info_box->pack_start(*keywords);

    button_box->set_halign(Gtk::ALIGN_END);
    button_box->set_valign(Gtk::ALIGN_CENTER);
    button_box->set_margin_bottom(10);
    button_box->set_margin_start(10);
    button_box->set_margin_top(10);
    button_box->set_margin_end(10);
    button_box->set_spacing(10);
    button_box->pack_start(*m_add_to_queue);
    button_box->pack_start(*m_install);
    button_box->pack_start(*m_uninstall);

    m_add_to_queue->set_image_from_icon_name("list-add-symbolic");
    m_add_to_queue->set_tooltip_text(
        std::format("Add {} to {} queue", m_pkg[PKG_NAME],
                    p_card_type == Type::INSTALL ? "install" : "uninstall"));

    m_install->set_image_from_icon_name("document-save-symbolic");
    m_install->set_tooltip_text(
        std::format("Install {} right now", m_pkg[PKG_NAME]));

    m_uninstall->set_image_from_icon_name("list-remove-symbolic");
    m_uninstall->set_tooltip_text(
        std::format("Uninstall {} right now", m_pkg[PKG_NAME]));

    main_box->pack_start(*info_box);
    main_box->pack_start(*button_box);

    m_card->set_halign(Gtk::ALIGN_FILL);
    m_card->set_margin_bottom(10);
    m_card->set_margin_start(10);
    m_card->set_margin_end(10);
    m_card->set_name("card-container");
    m_card->add(*main_box);
    m_card->show_all_children();
    m_card->set_visible(true);

    if (p_card_type == Card::Type::INSTALL)
    {
        m_uninstall->set_visible(false);
        m_install->set_visible(true);
    }
    else
    {
        m_uninstall->set_visible(true);
        m_install->set_visible(false);
    }
}


auto
Card::is_valid() -> bool
{
    return m_pkg.is_valid();
}


auto
Card::get_package() -> Package &
{
    return m_pkg;
}


auto
Card::get_widget() -> Gtk::Frame *
{
    return m_card;
}
