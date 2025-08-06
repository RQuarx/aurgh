#include <expected>

#include <gtkmm/comboboxtext.h>
#include <gtkmm/togglebutton.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/cssprovider.h>
#include <gtkmm/searchentry.h>
#include <gtkmm/window.h>
#include <gtkmm/box.h>
#include <glibmm/fileutils.h>
#include <glibmm/markup.h>
#include <curl/curl.h>

#include "app/utils.hh"
#include "aliases.hh"
#include "app/app.hh"
#include "log.hh"

using app::App;


App::App( const std::shared_ptr<Logger> &logger ) :
    m_app(Gtk::Application::create("org.kei.aurgh")),
    m_logger(logger)
{
    m_logger->log<INFO>("Starting application");

    /* Silent the 'Fontconfig warning: using without calling FcInit()'. */
    FcInit();

    auto _ { init_curl(CURL_VERSION_THREADSAFE).or_else(
    [this]( const std::string &err ) -> res_or_string<void>
    {
        m_logger->log<ERROR>("Failed to initialize CURL: {}", err);
        exit(1);
        return {};
    }) };

    auto builder { *app::get_builder("window.xml").or_else(
    [this]( const std::string &err ) -> res_or_string<builder_t>
    {
        m_logger->log<ERROR>("Failed to create a Gtk::Builder: {}", err);
        exit(1);
        return {};
    }) };

    load_css();

    m_logger->log<DEBUG>("Creating widgets");
    builder->get_widget("aurgh_window", m_window);

    builder->get_widget("aur_tab_button", m_aur_button);
    builder->get_widget("main_tab_button", m_main_button);
    builder->get_widget("installed_tab_button", m_installed_button);

    builder->get_widget("aurgh_search_by_combo", m_criteria.search_by);
    builder->get_widget("aurgh_sort_by_combo", m_criteria.sort_by);
    builder->get_widget("aurgh_reverse_check", m_criteria.reverse);

    builder->get_widget("aurgh_search_entry", m_criteria.search_entry);

    builder->get_widget("aurgh_content_box", m_content_box);

    m_aur_button->signal_clicked().connect(sigc::bind(
        sigc::mem_fun(*this, &App::on_tab_button_pressed), m_aur_button));
    m_main_button->signal_clicked().connect(sigc::bind(
        sigc::mem_fun(*this, &App::on_tab_button_pressed), m_main_button));
    m_installed_button->signal_clicked().connect(sigc::bind(
        sigc::mem_fun(*this, &App::on_tab_button_pressed),
        m_installed_button));

    setup_criteria();
    setup_tabs();

    m_content_box->foreach(
        [this](Gtk::Widget &w){ m_content_box->remove(w); });
    m_window->show_all_children();
}


auto
App::init_curl( int64_t flags ) const -> res_or_string<void>
{
    m_logger->log<INFO>("Initializing CURL.");
    CURLcode retval { curl_global_init(flags) };

    if (retval != CURLE_OK)
        return std::unexpected(curl_easy_strerror(retval));

    return {};
}


void
App::load_css( void ) const
{
    m_logger->log<INFO>("Loading CSS file.");
    auto css_provider { Gtk::CssProvider::create() };

    css_provider->signal_parsing_error().connect(
    [this]( const Glib::RefPtr<const Gtk::CssSection> &section,
            const Glib::Error                         &error ) -> void
    {
        if (!section) {
            m_logger->log<ERROR>("Failed to parse CSS file");
            throw std::runtime_error("");
        }
        std::string file_name  { section->get_file()->get_path() };
        uint32_t    start_line { section->get_start_line() + 1   };

        m_logger->log<ERROR>("Failed to parse {} at line {}: {}",
                              file_name, start_line, error.what().raw());
        throw std::runtime_error("");
    });

    css_provider->load_from_path(app::get_app_file("style.css"));

    auto screen { Gdk::Screen::get_default() };
    Gtk::StyleContext::add_provider_for_screen(
        screen, css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
}


auto
App::run( void ) -> int32_t
{ return m_app->run(*m_window); }


void
App::setup_tabs( void )
{
    m_logger->log<DEBUG>("Creating tabs");
    auto aur { Gtk::Builder::create_from_file(
               app::get_app_file("tabs/aur.xml")) };
    aur->get_widget_derived<aur::Tab>("aur_tab", m_aur_tab,
                                                 m_logger,
                                                 m_signal);
    m_content_box->pack_start(*m_aur_tab);
}


void
App::setup_criteria( void )
{
    m_criteria.search_by->signal_changed().connect(sigc::bind(
        sigc::mem_fun(*this, &App::on_criteria_change),
        SEARCH_COMBO, TAB_NONE
    ));
    m_criteria.sort_by->signal_changed().connect(sigc::bind(
        sigc::mem_fun(*this, &App::on_criteria_change),
        SORT_COMBO, TAB_NONE
    ));
    m_criteria.search_entry->signal_activate().connect(sigc::bind(
        sigc::mem_fun(*this, &App::on_criteria_change),
        SEARCH_ENTRY, TAB_NONE
    ));
    m_criteria.reverse->signal_clicked().connect(sigc::bind(
        sigc::mem_fun(*this, &App::on_criteria_change),
        REVERSE_CHECK, TAB_NONE
    ));

    m_criteria.search_by->set_active_id("name");
    m_criteria.sort_by->set_active_id("NumVotes");
}


void
App::on_tab_button_pressed( Gtk::ToggleButton *button )
{
    auto rm_child {
        [this]( Gtk::Widget &w ){ m_content_box->remove(w); } };

    if (button->get_active()) {
        if (button == m_aur_button) {
            m_main_button->set_active(false);
            m_installed_button->set_active(false);

            m_content_box->foreach(rm_child);
            m_content_box->pack_start(*m_aur_tab);
            m_content_box->show_all_children();
            on_criteria_change(CRITERIA_NONE, AUR);
        }
        else if (button == m_main_button) {
            m_aur_button->set_active(false);
            m_installed_button->set_active(false);

            m_content_box->foreach(rm_child);
            // m_content_box->pack_start(*m_main_tab);
            m_content_box->show_all_children();
            on_criteria_change(CRITERIA_NONE, MAIN);
        }
        else if (button == m_installed_button) {
            m_main_button->set_active(false);
            m_aur_button->set_active(false);

            m_content_box->foreach(rm_child);
            // m_content_box->pack_start(*m_installed_tab);
            m_content_box->show_all_children();
            on_criteria_change(CRITERIA_NONE, INSTALLED);
        }
    }
}


void
App::on_criteria_change( CriteriaType type, TabType tab )
{
    if (tab != TAB_NONE) {
        m_signal.emit(tab, m_criteria, type);
        return;
    }

    if (m_aur_button->get_active())
        m_signal.emit(AUR, m_criteria, type);
    else if (m_main_button->get_active())
        m_signal.emit(MAIN, m_criteria, type);
    else if (m_installed_button->get_active())
        m_signal.emit(INSTALLED, m_criteria, type);
}