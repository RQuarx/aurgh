#include <gtkmm.h>

#include "app/tabs/card.hh"
#include "app/package.hh"
#include "log.hh"

using app::Card;


Card::Card( const std::shared_ptr<Logger> &p_logger,
            const Json::Value             &p_pkg,
            const Type                    &p_card_type ) :
    m_logger(p_logger),
    m_pkg(m_logger, p_pkg),
    m_card(Gtk::make_managed<Gtk::Box>()),
    m_install(Gtk::make_managed<Gtk::ToggleButton>()),
    m_add_to_queue(Gtk::make_managed<Gtk::ToggleButton>()),
    m_uninstall(Gtk::make_managed<Gtk::ToggleButton>())
{
    auto *info_box   { Gtk::make_managed<Gtk::VBox>() };
    auto *button_box { Gtk::make_managed<Gtk::VBox>() };

    auto *name_ver { Gtk::make_managed<Gtk::Label>() };
    auto *desc     { Gtk::make_managed<Gtk::Label>() };
    auto *keywords { Gtk::make_managed<Gtk::Box>()   };

    name_ver->set_markup(std::format(
                        "<span size='xx-large'><b>{}</b></span>    {}",
                         m_pkg[PKG_NAME], m_pkg[PKG_VERSION]));
    name_ver->set_halign(Gtk::ALIGN_START);

    desc->set_markup(std::format("<big>{}</big>", m_pkg[PKG_DESC]));
    desc->set_halign(Gtk::ALIGN_START);
    desc->set_valign(Gtk::ALIGN_START);
    desc->set_line_wrap();

    keywords->set_spacing(10);
    keywords->set_halign(Gtk::ALIGN_START);

    for (const std::string &kw : m_pkg.get_keywords()) {
        auto frame { Gtk::make_managed<Gtk::Frame>() };
        auto label { Gtk::make_managed<Gtk::Label>(kw) };

        frame->set_hexpand(false);
        frame->set_halign(Gtk::ALIGN_START);
        frame->add(*label);
        keywords->pack_start(*frame);
    }

    info_box->set_spacing(10);
    info_box->pack_start(*name_ver);
    info_box->pack_start(*desc);
    info_box->pack_start(*keywords);

    button_box->set_halign(Gtk::ALIGN_END);
    button_box->set_orientation(Gtk::ORIENTATION_VERTICAL);
    button_box->pack_start(*m_add_to_queue);
    button_box->pack_start(*m_install);
    button_box->pack_start(*m_uninstall);

    m_add_to_queue->set_image_from_icon_name("list-add-symbolic");
    m_install->set_image_from_icon_name("document-save-symbolic");
    m_uninstall->set_image_from_icon_name("list-remove-symbolic");

    m_card->set_halign(Gtk::ALIGN_FILL);
    m_card->set_spacing(10);
    m_card->set_name("card-container");
    m_card->pack_start(*info_box);
    m_card->show_all_children();

    if (p_card_type == Card::INSTALL)
        m_uninstall->hide();
    else
        m_install->show();
}


auto
Card::is_valid( void ) -> bool
{ return m_pkg.is_valid(); }


Card::~Card( void )
{}


auto
Card::get_widget( void ) -> Gtk::Box *
{ return m_card; }