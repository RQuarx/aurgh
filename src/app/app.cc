#include <expected>

#include <curl/curl.h>
#include <glibmm.h>
#include <gtkmm.h>

#include "app/app.hh"
#include "app/utils.hh"
#include "log.hh"

using app::App;


App::App() : app(Gtk::Application::create("org.kei.aurgh"))
{
    logger.log<INFO>("Starting application");

    /* Silent the 'Fontconfig warning: using without calling FcInit()'
       warning.
    */
    FcInit();

    init_curl(CURL_VERSION_THREADSAFE);

    auto builder { app::get_builder("/org/kei/aurgh/data/window.xml") };

    load_css();

    logger.log<DEBUG>("Creating widgets");
    builder->get_widget("aurgh_window", window);

    builder->get_widget("sidebar_button", sidebar_button);
    builder->get_widget("aur_tab_button", aur_button);
    builder->get_widget("main_tab_button", main_button);
    builder->get_widget("installed_tab_button", installed_button);

    builder->get_widget("aurgh_search_by_combo", criteria.search_by);
    builder->get_widget("aurgh_sort_by_combo", criteria.sort_by);
    builder->get_widget("aurgh_reverse_check", criteria.reverse);

    builder->get_widget("aurgh_search_entry", criteria.search_entry);

    builder->get_widget("aurgh_sidebar_box", sidebar_box);
    builder->get_widget("aurgh_content_box", content_box);

    aur_button->signal_clicked().connect(sigc::bind(
        sigc::mem_fun(*this, &App::on_tab_button_pressed), aur_button));
    main_button->signal_clicked().connect(sigc::bind(
        sigc::mem_fun(*this, &App::on_tab_button_pressed), main_button));
    installed_button->signal_clicked().connect(sigc::bind(
        sigc::mem_fun(*this, &App::on_tab_button_pressed), installed_button));

    setup_criteria();
    setup_tabs();

    content_box->foreach([this](Gtk::Widget &w) -> void
                         { content_box->remove(w); });
    window->show_all_children();

    sidebar_button->set_visible(false);
    sidebar_box->set_visible(false);

    sidebar = std::make_unique<app::Sidebar>(sidebar_button, sidebar_box);
}


void
App::init_curl(std::int64_t flags)
{
    logger.log<INFO>("Initializing CURL.");
    CURLcode retval { curl_global_init(flags) };

    if (retval == CURLE_OK) return;

    logger.log<ERROR>("CURL failed to initialize: {}",
                      curl_easy_strerror(retval));
    std::terminate();
}


void
App::load_css()
{
    logger.log<INFO>("Loading CSS file.");
    auto css_provider { Gtk::CssProvider::create() };

    css_provider->signal_parsing_error().connect(
        [](const Glib::RefPtr<const Gtk::CssSection> &section,
           const Glib::Error                         &error) -> void
        {
            if (!section)
            {
                logger.log<ERROR>("Failed to parse CSS file");
                throw std::runtime_error("");
            }
            std::string   file_name { section->get_file()->get_path() };
            std::uint32_t start_line { section->get_start_line() + 1 };

            logger.log<ERROR>("Failed to parse {} at line {}: {}", file_name,
                              start_line, error.what().raw());
            throw std::runtime_error("");
        });

    css_provider->load_from_resource("/org/kei/aurgh/data/style.css");

    auto screen { Gdk::Screen::get_default() };
    Gtk::StyleContext::add_provider_for_screen(
        screen, css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
}


auto
App::run(this App &self) -> int
{
    return self.app->run(*self.window);
}


void
App::setup_tabs(this App &self)
{
    logger.log<DEBUG>("Creating tabs");
    auto aur { get_builder("/org/kei/aurgh/data/tabs/aur.xml") };
    aur->get_widget_derived<aur::Tab>("aur_tab", self.aur_tab);
    self.content_box->pack_start(*self.aur_tab);
}


void
App::setup_criteria()
{
    this->criteria.search_by->signal_changed().connect(
        sigc::bind(sigc::mem_fun(*this, &App::on_criteria_change),
                   CriteriaType::SEARCH_BY, TabType::NONE));

    this->criteria.sort_by->signal_changed().connect(
        sigc::bind(sigc::mem_fun(*this, &App::on_criteria_change),
                   CriteriaType::SORT_BY, TabType::NONE));

    this->criteria.search_entry->signal_activate().connect(
        sigc::bind(sigc::mem_fun(*this, &App::on_criteria_change),
                   CriteriaType::SEARCH_TEXT, TabType::NONE));
    this->criteria.reverse->signal_clicked().connect(
        sigc::bind(sigc::mem_fun(this, &App::on_criteria_change),
                   CriteriaType::REVERSE, TabType::NONE));

    this->criteria.search_by->set_active_id("name");
    this->criteria.sort_by->set_active_id("NumVotes");
}


void
App::on_tab_button_pressed(Gtk::ToggleButton *button)
{
    auto rm_child { [this](Gtk::Widget &w) -> void
                    { content_box->remove(w); } };

    if (button->get_active())
    {
        if (button == this->aur_button)
        {
            this->main_button->set_active(false);
            this->installed_button->set_active(false);
            // this->main_tab->close();
            // this->installed_tab->close();

            this->content_box->foreach(rm_child);
            this->content_box->pack_start(*this->aur_tab);
            this->content_box->show_all_children();
            this->on_criteria_change(CriteriaType::NONE, TabType::AUR);
        }
        else if (button == this->main_button)
        {
            this->aur_button->set_active(false);
            this->installed_button->set_active(false);
            this->aur_tab->close();
            // this->installed_tab->close();

            this->content_box->foreach(rm_child);
            // this->content_box->pack_start(*this->main_tab);
            this->content_box->show_all_children();
            this->on_criteria_change(CriteriaType::NONE, TabType::MAIN);
        }
        else if (button == this->installed_button)
        {
            this->main_button->set_active(false);
            this->aur_button->set_active(false);
            // this->main_tab->close();
            this->aur_tab->close();

            this->content_box->foreach(rm_child);
            // this->content_box->pack_start(*this->installed_tab);
            this->content_box->show_all_children();
            this->on_criteria_change(CriteriaType::NONE, TabType::INSTALLED);
        }
    }
}


void
App::on_criteria_change(CriteriaType type, TabType tab)
{
#define CONDITION(p_tab_type, p_button) \
     tab == (p_tab_type) || (tab == TabType::NONE && (p_button)->get_active())

    if (CONDITION(TabType::AUR, this->aur_button))
        aur_tab->activate(this->criteria, type);
    else if (CONDITION(TabType::MAIN, this->main_button))
        return; /* NOLINT */
    else if (CONDITION(TabType::INSTALLED, this->installed_button))
        return;

#undef CONDITION
}
