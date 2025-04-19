#include "logger.hh"
#include "window.hh"
#include <gtkmm-3.0/gtkmm/settings.h>
#include <gtkmm-3.0/gtkmm/icontheme.h>
#include <gtkmm-3.0/gtkmm/stock.h>
#include <gtkmm-3.0/gtkmm/label.h>
#include <gtkmm-3.0/gtkmm/button.h>
#include <gdkmm/pixbuf.h>
#include <filesystem>


AURWindow::AURWindow(std::array<Gtk::Box*, 2> tabs, Logger *logger) :
    m_tabs(tabs),
    m_notebook(Gtk::make_managed<Gtk::Notebook>()),
    m_header_bar(Gtk::make_managed<Gtk::HeaderBar>()),
    m_css_provider(Gtk::CssProvider::create()),
    m_logger(logger)
{
    set_title("AUR Gtk Helper");
    set_default_size(800, 600);

    // Use system theme settings
    auto settings = Gtk::Settings::get_default();
    if (settings) {
        // Enable theme-provided icons
        settings->property_gtk_menu_images() = true;
        settings->property_gtk_button_images() = true;
    }

    // Load CSS styling (minimal, theme-respecting)
    load_css();

    // Setup header bar
    setup_header_bar();

    // Setup notebook with icons
    setup_notebook_icons();

    add(*m_notebook);
    show_all();
}


void
AURWindow::load_css()
{
    try {
        std::string css_path = "src/style.css";
        if (std::filesystem::exists(css_path)) {
            m_css_provider->load_from_path(css_path);
            m_logger->log(Logger::Debug, "Loaded CSS from {}", css_path.c_str());
        } else {
            m_logger->log(Logger::Warn, "CSS file not found at {}", css_path.c_str());
            return;
        }

        // Apply CSS to the application
        auto screen = Gdk::Screen::get_default();
        Gtk::StyleContext::add_provider_for_screen(
            screen, m_css_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
        );
    } catch (const Glib::Error& e) {
        m_logger->log(Logger::Error, "Failed to load CSS: {}", e.what().c_str());
    }
}


void
AURWindow::setup_header_bar()
{
    m_header_bar->set_show_close_button(true);
    m_header_bar->set_title("AURGH");
    m_header_bar->set_subtitle("AUR Graphical Helper");

    // Set application icon
    try {
        std::string icon_path = "src/app-icon.svg";
        if (std::filesystem::exists(icon_path)) {
            auto app_icon = Gdk::Pixbuf::create_from_file(icon_path);
            set_icon(app_icon);

            // Add app icon to header bar (optional - many themes already show the app icon)
            auto app_image = Gtk::make_managed<Gtk::Image>();
            app_image->set(app_icon->scale_simple(24, 24, Gdk::INTERP_BILINEAR));
            // m_header_bar->pack_start(*app_image); // Commented out to respect theme

            m_logger->log(Logger::Debug, "Loaded application icon from {}", icon_path.c_str());
        } else {
            m_logger->log(Logger::Warn, "Application icon not found at {}", icon_path.c_str());
        }
    } catch (const Glib::Error& e) {
        m_logger->log(Logger::Error, "Failed to load application icon: {}", e.what().c_str());
    }

    // Add a refresh button with icon - using theme-compatible styling
    auto *refresh_button = Gtk::make_managed<Gtk::Button>();
    auto *refresh_icon = Gtk::make_managed<Gtk::Image>();
    refresh_icon->set_from_icon_name("view-refresh-symbolic", Gtk::ICON_SIZE_BUTTON);
    refresh_button->set_image(*refresh_icon);
    refresh_button->set_tooltip_text("Refresh");
    refresh_button->set_relief(Gtk::RELIEF_NONE); // More theme-compatible
    m_header_bar->pack_end(*refresh_button);

    // Add an about button with icon
    auto *about_button = Gtk::make_managed<Gtk::Button>();
    auto *about_icon = Gtk::make_managed<Gtk::Image>();
    about_icon->set_from_icon_name("help-about-symbolic", Gtk::ICON_SIZE_BUTTON);
    about_button->set_image(*about_icon);
    about_button->set_tooltip_text("About");
    about_button->set_relief(Gtk::RELIEF_NONE); // More theme-compatible
    m_header_bar->pack_end(*about_button);

    // Add a settings button with icon
    auto *settings_button = Gtk::make_managed<Gtk::Button>();
    auto *settings_icon = Gtk::make_managed<Gtk::Image>();
    settings_icon->set_from_icon_name("preferences-system-symbolic", Gtk::ICON_SIZE_BUTTON);
    settings_button->set_image(*settings_icon);
    settings_button->set_tooltip_text("Settings");
    settings_button->set_relief(Gtk::RELIEF_NONE); // More theme-compatible
    m_header_bar->pack_end(*settings_button);

    set_titlebar(*m_header_bar);
}


void
AURWindow::setup_notebook_icons()
{
    // Create tabs with icons
    m_notebook->remove_page(0);
    m_notebook->remove_page(0);

    // Packages tab with icon
    auto packages_icon = Gtk::make_managed<Gtk::Image>();
    packages_icon->set_from_icon_name("system-software-install-symbolic", Gtk::ICON_SIZE_MENU);
    auto packages_label = Gtk::make_managed<Gtk::Label>("Packages");
    auto packages_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 5);
    packages_box->pack_start(*packages_icon, false, false);
    packages_box->pack_start(*packages_label, false, false);
    packages_box->show_all();

    // Management tab with icon
    auto management_icon = Gtk::make_managed<Gtk::Image>();
    management_icon->set_from_icon_name("preferences-system-symbolic", Gtk::ICON_SIZE_MENU);
    auto management_label = Gtk::make_managed<Gtk::Label>("Management");
    auto management_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 5);
    management_box->pack_start(*management_icon, false, false);
    management_box->pack_start(*management_label, false, false);
    management_box->show_all();

    // Add tabs with icons
    m_notebook->append_page(*m_tabs.at(0), *packages_box);
    m_notebook->append_page(*m_tabs.at(1), *management_box);
}