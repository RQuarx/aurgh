#include <gtkmm/scrolledwindow.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/grid.h>
#include <gtkmm/box.h>
#include "app/tabs/card.hh"
#include "app/package.hh"
#include "log.hh"


namespace
{
    auto
    escape_pango_markup( const std::string &p_text ) -> std::string
    {
        std::ostringstream out;
        for (char ch : p_text) {
            switch (ch) {
                case '&': out << "&amp;"; break;
                case '<': out << "&lt;"; break;
                case '>': out << "&gt;"; break;
                default:  out << ch; break;
            }
        }
        return out.str();
    }
}


Card::Card( const std::shared_ptr<Logger> &logger,
            const Json::Value             &pkg ) :
    m_logger(logger),
    m_pkg(m_logger, pkg),
    m_card(Gtk::make_managed<Gtk::Box>())
{
    auto *info_box { Gtk::make_managed<Gtk::VBox>() };
    auto *button_box { Gtk::make_managed<Gtk::VBox>() };

    auto *name_ver { Gtk::make_managed<Gtk::Label>() };
    auto *desc { Gtk::make_managed<Gtk::Label>() };
    auto *keywords { Gtk::make_managed<Gtk::Box>() };

    name_ver->set_markup(std::format(
                        "<span size='xx-large'><b>{}</b></span>    {}",
                         m_pkg[PKG_NAME], m_pkg[PKG_VERSION]));
    name_ver->set_halign(Gtk::ALIGN_START);

    desc->set_markup(std::format("<big>{}</big>",
                                  escape_pango_markup(m_pkg[PKG_DESC])));
    desc->set_halign(Gtk::ALIGN_START);
    desc->set_valign(Gtk::ALIGN_START);
    desc->set_line_wrap();

    keywords->set_spacing(10);
    keywords->set_halign(Gtk::ALIGN_START);
    std::ranges::for_each(m_pkg.get_keywords(),
    [keywords]( const std::string &kw )
    {
        auto frame { Gtk::make_managed<Gtk::Frame>() };
        auto label { Gtk::make_managed<Gtk::Label>(kw) };

        frame->set_hexpand(false);
        frame->set_halign(Gtk::ALIGN_START);
        frame->add(*label);
        keywords->pack_start(*frame);
    });

    info_box->set_spacing(10);
    info_box->pack_start(*name_ver);
    info_box->pack_start(*desc);
    info_box->pack_start(*keywords);

    button_box->set_halign(Gtk::ALIGN_END);

    m_card->set_halign(Gtk::ALIGN_FILL);
    m_card->set_spacing(10);
    m_card->set_name("card-container");
    m_card->pack_start(*info_box);
}


auto
Card::is_valid( void ) -> bool
{ return m_pkg.is_valid(); }


Card::~Card( void )
{}


auto
Card::get_widget( void ) -> Gtk::Box *
{ return m_card; }