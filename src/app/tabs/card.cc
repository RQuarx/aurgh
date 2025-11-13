#include <gtkmm.h>

#include "app/package.hh"
#include "app/tabs/card.hh"

using app::Card;


Card::Card(const Json::Value &p_pkg, const Type &p_card_type)
    : pkg(p_pkg), card(Gtk::make_managed<Gtk::Frame>()),
      install(Gtk::make_managed<Gtk::ToggleButton>()),
      add_to_queue(Gtk::make_managed<Gtk::ToggleButton>()),
      uninstall(Gtk::make_managed<Gtk::ToggleButton>())
{
    auto *main_box { Gtk::make_managed<Gtk::Box>() };

    auto *info_box { Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL) };
    auto *button_box { Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL) };

    auto *name_ver { Gtk::make_managed<Gtk::Label>() };
    auto *desc { Gtk::make_managed<Gtk::Label>() };
    auto *keywords { Gtk::make_managed<Gtk::Box>() };

    name_ver->set_markup(
        std::format("<span size='xx-large'><b>{}</b></span>    {}",
                    pkg[PKG_NAME], pkg[PKG_VERSION]));
    name_ver->set_halign(Gtk::ALIGN_START);

    desc->set_markup(std::format("<big>{}</big>", pkg[PKG_DESC]));
    desc->set_halign(Gtk::ALIGN_START);
    desc->set_valign(Gtk::ALIGN_START);
    desc->set_line_wrap();

    keywords->set_spacing(10);
    keywords->set_halign(Gtk::ALIGN_START);

    for (const std::string &kw : pkg.get_keywords())
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
    button_box->pack_start(*add_to_queue);
    button_box->pack_start(*install);
    button_box->pack_start(*uninstall);

    add_to_queue->set_image_from_icon_name("list-add-symbolic");
    add_to_queue->set_tooltip_text(
        std::format("Add {} to {} queue", pkg[PKG_NAME],
                    p_card_type == Type::INSTALL ? "install" : "uninstall"));

    install->set_image_from_icon_name("document-save-symbolic");
    install->set_tooltip_text(
        std::format("Install {} right now", pkg[PKG_NAME]));

    uninstall->set_image_from_icon_name("list-remove-symbolic");
    uninstall->set_tooltip_text(
        std::format("Uninstall {} right now", pkg[PKG_NAME]));

    main_box->pack_start(*info_box);
    main_box->pack_start(*button_box);

    card->set_halign(Gtk::ALIGN_FILL);
    card->set_margin_bottom(10);
    card->set_margin_start(10);
    // card->set_margin_top(10);
    card->set_margin_end(10);
    card->set_name("card-container");
    card->add(*main_box);
    card->show_all_children();
    card->set_visible(true);

    if (p_card_type == Card::Type::INSTALL)
    {
        uninstall->set_visible(false);
        install->set_visible(true);
    }
    else
    {
        uninstall->set_visible(true);
        install->set_visible(false);
    }
}


auto
Card::is_valid() -> bool
{
    return pkg.is_valid();
}


Card::~Card() {}


auto
Card::get_widget() -> Gtk::Frame *
{
    return card;
}
