#include <curl/curl.h>
#include <glibmm.h>
#include <gtkmm.h>

#include "app/app.hh"
#include "app/utils.hh"
#include "logger.hh"

using app::App;


App::App() : m_app(Gtk::Application::create(APP_ID))
{
    logger[Level::INFO, "app"]("Starting application");

    /* Silent the 'Fontconfig warning: using without calling FcInit()'
       warning.
    */
    FcInit();

    init_curl(CURL_VERSION_THREADSAFE);

    auto builder { app::get_builder("/org/kei/aurgh/data/window.xml") };

    load_css();

    logger[Level::DEBUG, "app"]("Creating widgets");
    builder->get_widget("aurgh_window", m_window);

    builder->get_widget("sidebar_button", m_sidebar_button);
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
        sigc::mem_fun(*this, &App::on_tab_button_pressed), m_installed_button));

    setup_criteria();
    setup_tabs();

    m_content_box->foreach([this](Gtk::Widget &w) -> void
                           { m_content_box->remove(w); });
    m_window->show_all_children();

    m_sidebar_button->set_visible(false);

    builder->get_widget_derived<Sidebar>("aurgh_sidebar_box", m_sidebar,
                                         m_sidebar_button);

    m_sidebar->add_queue_signal(m_aur_tab->signal_queue_update());

    m_sidebar->show_all_children();
    m_sidebar->set_visible(false);
}


void
App::init_curl(std::int64_t flags)
{
    logger[Level::DEBUG, "app"]("Initializing CURL");
    CURLcode retval { curl_global_init(flags) };

    if (retval == CURLE_OK) return;

    logger[Level::FATAL, "app"]("CURL failed to initialize: {}",
                                curl_easy_strerror(retval));
    std::terminate();
}


void
App::load_css()
{
    logger[Level::DEBUG, "app"]("Loading CSS file");
    auto css_provider { Gtk::CssProvider::create() };

    css_provider->signal_parsing_error().connect(
        [](const Glib::RefPtr<const Gtk::CssSection> &section,
           const Glib::Error                         &error) -> void
        {
            if (!section)
            {
                logger[Level::ERROR, "app::css"]("Failed to parse CSS file");
                throw std::runtime_error { "" };
            }
            std::string   file_name { section->get_file()->get_path() };
            std::uint32_t start_line { section->get_start_line() + 1 };

            logger[Level::ERROR, "app::css"](
                "Failed to parse {} at line {}: {}", file_name, start_line,
                error.what().raw());

            throw std::runtime_error { "" };
        });

    css_provider->load_from_resource("/org/kei/aurgh/data/style.css");

    auto screen { Gdk::Screen::get_default() };
    Gtk::StyleContext::add_provider_for_screen(
        screen, css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
}


auto
App::run() -> int
{
    return m_app->run(*m_window);
}


void
App::setup_tabs()
{
    logger[Level::DEBUG, "app"]("Creating tabs");
    auto aur { get_builder("/org/kei/aurgh/data/tabs/aur.xml") };
    aur->get_widget_derived<aur::Tab>("aur_tab", m_aur_tab);
    m_content_box->pack_start(*m_aur_tab);
}


void
App::setup_criteria()
{
    m_criteria.search_by->signal_changed().connect(
        sigc::bind(sigc::mem_fun(*this, &App::on_criteria_change),
                   CriteriaType::SEARCH_BY, TabType::NONE));

    m_criteria.sort_by->signal_changed().connect(
        sigc::bind(sigc::mem_fun(*this, &App::on_criteria_change),
                   CriteriaType::SORT_BY, TabType::NONE));

    m_criteria.search_entry->signal_activate().connect(
        sigc::bind(sigc::mem_fun(*this, &App::on_criteria_change),
                   CriteriaType::SEARCH_TEXT, TabType::NONE));
    m_criteria.reverse->signal_clicked().connect(
        sigc::bind(sigc::mem_fun(this, &App::on_criteria_change),
                   CriteriaType::REVERSE, TabType::NONE));

    m_criteria.search_by->set_active_id("name");
    m_criteria.sort_by->set_active_id("NumVotes");
}


void
App::on_tab_button_pressed(Gtk::ToggleButton *button)
{
    auto rm_child { [this](Gtk::Widget &w) -> void
                    { m_content_box->remove(w); } };

    if (button->get_active())
    {
        if (button == m_aur_button)
        {
            m_main_button->set_active(false);
            m_installed_button->set_active(false);
            // main_tab->close();
            // installed_tab->close();

            m_content_box->foreach(rm_child);
            m_content_box->pack_start(*m_aur_tab);
            m_content_box->show_all_children();
            on_criteria_change(CriteriaType::NONE, TabType::AUR);
        }
        else if (button == m_main_button)
        {
            m_aur_button->set_active(false);
            m_installed_button->set_active(false);
            m_aur_tab->close();
            // installed_tab->close();

            m_content_box->foreach(rm_child);
            // m_content_box->pack_start(*main_tab);
            m_content_box->show_all_children();
            on_criteria_change(CriteriaType::NONE, TabType::MAIN);
        }
        else if (button == m_installed_button)
        {
            m_main_button->set_active(false);
            m_aur_button->set_active(false);
            // main_tab->close();
            m_aur_tab->close();

            m_content_box->foreach(rm_child);
            // m_content_box->pack_start(*installed_tab);
            m_content_box->show_all_children();
            on_criteria_change(CriteriaType::NONE, TabType::INSTALLED);
        }
    }
}


void
App::on_criteria_change(CriteriaType type, TabType tab)
{
#define CONDITION(tab_type, button) \
     tab == (tab_type) || (tab == TabType::NONE && (button)->get_active())

    if (CONDITION(TabType::AUR, m_aur_button))
        m_aur_tab->activate(m_criteria, type);
    else if (CONDITION(TabType::MAIN, m_main_button))
        return; /* NOLINT */
    else if (CONDITION(TabType::INSTALLED, m_installed_button))
        return;

#undef CONDITION
}
