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

    auto _ {
        init_curl(CURL_VERSION_THREADSAFE)
            .or_else(
                [](std::string_view err) -> std::expected<void, std::string>
                {
                    logger.log<ERROR>("Failed to initialize CURL: {}", err);
                    std::terminate();
                })
    };

    auto builder {
        *app::get_builder("window.xml")
             .or_else(
                 [](std::string_view err)
                     -> std::expected<Glib::RefPtr<Gtk::Builder>, std::string>
                 {
                     logger.log<ERROR>("Failed to create a Gtk::Builder: {}",
                                       err);
                     std::terminate();
                 })
    };

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

    // sidebar_button->set_visible(false);
    sidebar_box->set_visible(false);

    sidebar = std::make_unique<app::Sidebar>(sidebar_button, sidebar_box);
}


auto
App::init_curl(int64_t flags) -> std::expected<void, std::string>
{
    logger.log<INFO>("Initializing CURL.");
    CURLcode retval { curl_global_init(flags) };

    if (retval != CURLE_OK) return std::unexpected(curl_easy_strerror(retval));

    return {};
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
            std::string file_name { section->get_file()->get_path() };
            uint32_t    start_line { section->get_start_line() + 1 };

            logger.log<ERROR>("Failed to parse {} at line {}: {}", file_name,
                              start_line, error.what().raw());
            throw std::runtime_error("");
        });

    css_provider->load_from_path(app::get_app_file("style.css"));

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
    auto aur { Gtk::Builder::create_from_file(
        app::get_app_file("tabs/aur.xml")) };
    aur->get_widget_derived<aur::Tab>("aur_tab", self.aur_tab, self.signal);
    self.content_box->pack_start(*self.aur_tab);
}


void
App::setup_criteria()
{
    this->criteria.search_by->signal_changed().connect(
        sigc::bind(sigc::mem_fun(*this, &App::on_criteria_change), SEARCH_COMBO,
                   TAB_NONE));
    this->criteria.sort_by->signal_changed().connect(sigc::bind(
        sigc::mem_fun(*this, &App::on_criteria_change), SORT_COMBO, TAB_NONE));
    this->criteria.search_entry->signal_activate().connect(
        sigc::bind(sigc::mem_fun(*this, &App::on_criteria_change), SEARCH_ENTRY,
                   TAB_NONE));
    this->criteria.reverse->signal_clicked().connect(
        sigc::bind(sigc::mem_fun(this, &App::on_criteria_change), REVERSE_CHECK,
                   TAB_NONE));

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

            this->content_box->foreach(rm_child);
            this->content_box->pack_start(*this->aur_tab);
            this->content_box->show_all_children();
            this->on_criteria_change(CRITERIA_NONE, AUR);
        }
        else if (button == this->main_button)
        {
            this->aur_button->set_active(false);
            this->installed_button->set_active(false);

            this->content_box->foreach(rm_child);
            // this->content_box->pack_start(*this->main_tab);
            this->content_box->show_all_children();
            this->on_criteria_change(CRITERIA_NONE, MAIN);
        }
        else if (button == this->installed_button)
        {
            this->main_button->set_active(false);
            this->aur_button->set_active(false);

            this->content_box->foreach(rm_child);
            // this->content_box->pack_start(*this->installed_tab);
            this->content_box->show_all_children();
            this->on_criteria_change(CRITERIA_NONE, INSTALLED);
        }
    }
}


void
App::on_criteria_change(CriteriaType type, TabType tab)
{
    if (tab != TAB_NONE)
    {
        this->signal.emit(tab, this->criteria, type);
        return;
    }

    if (this->aur_button->get_active())
        this->signal.emit(AUR, this->criteria, type);
    else if (this->main_button->get_active())
        this->signal.emit(MAIN, this->criteria, type);
    else if (this->installed_button->get_active())
        this->signal.emit(INSTALLED, this->criteria, type);
}
