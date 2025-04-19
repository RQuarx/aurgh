#include <gtkmm-3.0/gtkmm/frame.h>
#include <gtkmm-3.0/gtkmm/spinner.h>
#include <pangomm/layout.h>
#include <format>
#include <string>
#include "tabs/packages.hh"
#include "aur_client.hh"
#include "logger.hh"
#include "utils.hh"


PackageTab::PackageTab(AUR_Client *aur_client, Logger *logger) :
    m_search_results(Gtk::make_managed<Gtk::ScrolledWindow>()),
    m_search_by_combo(Gtk::make_managed<Gtk::ComboBoxText>()),
    m_entry(Gtk::make_managed<Gtk::SearchEntry>()),
    m_aur_client(aur_client),
    m_logger(logger)
{
    m_logger->log(Logger::Debug, "Creating packages tab");
    GtkUtils::set_margin(*this, 5);
    set_orientation(Gtk::ORIENTATION_VERTICAL);

    auto *frame = Gtk::make_managed<Gtk::Frame>();
    frame->add(*create_search_box());
    pack_start(*frame, false, false);

    auto *results_box = Gtk::make_managed<Gtk::Box>();
    m_search_results->set_placement(Gtk::CORNER_TOP_RIGHT);

    Gtk::Label *label = GtkUtils::create_label_markup("<b>Search to view AUR packages</b>");
    label->set_opacity(0.5);
    auto *info_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 5);
    info_box->pack_start(*label);
    info_box->set_halign(Gtk::ALIGN_CENTER);
    m_search_results->add(*info_box);

    results_box->pack_start(*m_search_results, true, true);
    pack_start(*results_box, true, true);

    show_all_children();
}


auto
PackageTab::create_search_box() -> Gtk::Box*
{
    auto *main_box = Gtk::make_managed<Gtk::Box>();

    // Add search icon
    auto *search_icon = Gtk::make_managed<Gtk::Image>();
    search_icon->set_from_icon_name("edit-find-symbolic", Gtk::ICON_SIZE_BUTTON);
    search_icon->set_margin_right(8);
    main_box->pack_start(*search_icon, false, false);

    // Setup search combo box with styling
    auto search_by_keywords = AUR_Client::get_search_by_keywords();
    for (const auto &s : search_by_keywords) {
        m_search_by_combo->append(s);
    }
    m_search_by_combo->set_active_text(search_by_keywords.at(0));
    m_search_by_combo->signal_changed().connect(
        sigc::mem_fun(*this, &PackageTab::on_search)
    );
    m_search_by_combo->set_tooltip_text("Select search criteria");
    main_box->pack_start(*m_search_by_combo, false, true);

    // Add a separator
    auto *separator = Gtk::make_managed<Gtk::Separator>(Gtk::ORIENTATION_VERTICAL);
    separator->set_margin_left(5);
    separator->set_margin_right(5);
    main_box->pack_start(*separator, false, false);

    // Setup search entry with styling
    m_entry->set_placeholder_text("Search AUR packages...");
    m_entry->signal_search_changed().connect(
        sigc::mem_fun(*this, &PackageTab::on_search)
    );
    m_entry->set_tooltip_text("Enter at least 3 characters to search");
    main_box->pack_start(*m_entry, true, true);

    // Add clear button with icon
    auto *clear_button = Gtk::make_managed<Gtk::Button>();
    auto *clear_icon = Gtk::make_managed<Gtk::Image>();
    clear_icon->set_from_icon_name("edit-clear-symbolic", Gtk::ICON_SIZE_BUTTON);
    clear_button->set_image(*clear_icon);
    clear_button->set_tooltip_text("Clear search");
    clear_button->set_margin_left(5);
    clear_button->signal_clicked().connect([this]() {
        m_entry->set_text("");
        m_search_results->remove();

        // Show the initial info message
        Gtk::Label *label = GtkUtils::create_label_markup("<b>Search to view AUR packages</b>");
        label->set_opacity(0.5);
        auto *info_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 5);
        info_box->pack_start(*label);
        info_box->set_halign(Gtk::ALIGN_CENTER);
        m_search_results->add(*info_box);
        m_search_results->show_all_children();
    });
    main_box->pack_start(*clear_button, false, false);

    GtkUtils::set_margin(*main_box, 8);
    return main_box;
}


void
PackageTab::on_search()
{
    if (m_entry->get_text_length() < 3) return;
    m_search_results->remove();

    std::string package_name = m_entry->get_text();
    std::string search_by = m_search_by_combo->get_active_text();
    m_logger->log(
        Logger::Info,
        "Searching for {}, by {}",
        package_name, search_by
    );

    // Show loading indicator
    auto *loading_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 10);
    loading_box->get_style_context()->add_class("loading-box");
    auto *spinner = Gtk::make_managed<Gtk::Spinner>();
    spinner->start();
    auto *loading_label = GtkUtils::create_label_markup("<i>Searching for packages...</i>");
    loading_box->pack_start(*spinner, false, false);
    loading_box->pack_start(*loading_label, false, false);
    loading_box->set_halign(Gtk::ALIGN_CENTER);
    loading_box->set_valign(Gtk::ALIGN_CENTER);
    m_search_results->add(*loading_box);
    m_search_results->show_all_children();

    // Process the search
    auto aur_packages = m_aur_client->search(package_name, search_by);
    m_search_results->remove();

    // Create a box for the results
    auto *results_container = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 0);
    results_container->get_style_context()->add_class("search-results-container");

    if (aur_packages.empty() || aur_packages["type"].asString() == "error") {
        // Show no results message
        auto *no_results = GtkUtils::create_label_markup("<i>No packages found</i>");
        no_results->set_opacity(0.7);
        auto *no_results_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 5);
        auto *no_results_icon = Gtk::make_managed<Gtk::Image>();
        no_results_icon->set_from_icon_name("dialog-information-symbolic", Gtk::ICON_SIZE_MENU);
        no_results_box->pack_start(*no_results_icon, false, false);
        no_results_box->pack_start(*no_results, false, false);
        no_results_box->set_halign(Gtk::ALIGN_CENTER);
        results_container->pack_start(*no_results_box, true, true);
    } else {
        // Add results count header
        int result_count = aur_packages["resultcount"].asInt();
        auto *results_header = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 5);
        results_header->get_style_context()->add_class("results-count");
        auto *results_count = GtkUtils::create_label_markup(
            std::format("<b>Found {} packages</b>", std::to_string(result_count))
        );
        results_header->pack_start(*results_count, false, false);
        results_container->pack_start(*results_header, false, false);

        // Create a grid for package results
        auto *packages_grid = Gtk::make_managed<Gtk::Grid>();
        packages_grid->set_column_spacing(10);
        packages_grid->set_row_spacing(10);
        packages_grid->set_column_homogeneous(true);

        // Add each package to the grid
        int row = 0, col = 0;
        for (auto package : aur_packages["results"]) {
            // Debug log the package data
            if (package.isMember("Name")) {
                m_logger->log(Logger::Debug, "Package name: {}", package["Name"].asString().c_str());
            } else {
                m_logger->log(Logger::Warn, "Package missing Name field");
            }

            if (package.isMember("Description")) {
                m_logger->log(Logger::Debug, "Package description: {}", package["Description"].asString().c_str());
            }

            auto *package_widget = create_package_widget(package);
            packages_grid->attach(*package_widget, col, row, 1, 1);

            // Move to next column or row
            col++;
            if (col >= 2) { // 2 columns layout
                col = 0;
                row++;
            }
        }

        results_container->pack_start(*packages_grid, true, true);
    }

    // Add the results to the scrolled window
    GtkUtils::set_margin(*results_container, 10);
    m_search_results->add(*results_container);
    m_search_results->show_all_children();
}


auto
PackageTab::create_package_widget(const Json::Value &package) -> Gtk::Widget*
{
    // Create a frame for the package info
    auto *package_frame = Gtk::make_managed<Gtk::Frame>();
    package_frame->set_shadow_type(Gtk::SHADOW_OUT);

    // Create the main container box
    auto *package_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 8);
    package_box->get_style_context()->add_class("package-info");
    package_box->set_border_width(8); // Add padding inside the frame

    // Package name with icon
    auto *name_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 5);
    auto *package_icon = Gtk::make_managed<Gtk::Image>();
    package_icon->set_from_icon_name("package-x-generic-symbolic", Gtk::ICON_SIZE_MENU);
    name_box->pack_start(*package_icon, false, false);

    // Make sure the package has a Name field
    if (!package.isMember("Name")) {
        m_logger->log(Logger::Error, "Package missing Name field");
        auto *error_label = GtkUtils::create_label_markup("<i>Invalid package data</i>");
        package_box->pack_start(*error_label, true, true);
        package_frame->add(*package_box);
        return package_frame;
    }

    std::string package_name = package["Name"].asString();
    auto *name_label = Gtk::make_managed<Gtk::Label>();
    name_label->set_markup(std::format("<b>{}</b>", package_name.c_str()));
    name_label->set_xalign(0.0); // Left align
    name_box->pack_start(*name_label, false, false);

    // Version info
    if (package.isMember("Version")) {
        std::string version = package["Version"].asString();
        auto *version_label = Gtk::make_managed<Gtk::Label>();
        version_label->set_markup(std::format("<span size='small' alpha='80%'>{}</span>", version.c_str()));
        version_label->set_margin_left(5);
        name_box->pack_start(*version_label, false, false);
    }

    package_box->pack_start(*name_box, false, false);

    // Description if available
    if (package.isMember("Description")) {
        std::string description = package["Description"].asString();
        if (description.length() > 80) {
            description = description.substr(0, 77) + "...";
        }
        auto *desc_label = Gtk::make_managed<Gtk::Label>();
        desc_label->set_markup(std::format("<span alpha='70%'>{}</span>", description.c_str()));
        desc_label->set_line_wrap(true);
        desc_label->set_line_wrap_mode(Pango::WRAP_WORD);
        desc_label->set_max_width_chars(40);
        desc_label->set_xalign(0.0); // Left align
        package_box->pack_start(*desc_label, false, false);
    }

    // Action buttons
    auto *action_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 5);
    action_box->set_halign(Gtk::ALIGN_END);

    // Info button
    auto *info_button = Gtk::make_managed<Gtk::Button>();
    info_button->get_style_context()->add_class("action-button");
    auto *info_icon = Gtk::make_managed<Gtk::Image>();
    info_icon->set_from_icon_name("help-about-symbolic", Gtk::ICON_SIZE_MENU);
    info_button->set_image(*info_icon);
    info_button->set_tooltip_text("View package details");
    info_button->set_relief(Gtk::RELIEF_NONE); // More theme-compatible
    info_button->signal_clicked().connect(
        sigc::bind(sigc::mem_fun(*this, &PackageTab::on_package_click), package_name)
    );
    action_box->pack_start(*info_button, false, false);

    // Install button
    auto *install_button = Gtk::make_managed<Gtk::Button>();
    install_button->get_style_context()->add_class("action-button");
    auto *install_icon = Gtk::make_managed<Gtk::Image>();
    // Try different icon names that might work better with dark themes
    install_icon->set_from_icon_name("list-add-symbolic", Gtk::ICON_SIZE_MENU);
    install_button->set_image(*install_icon);
    install_button->set_tooltip_text("Install package");
    install_button->set_relief(Gtk::RELIEF_NONE); // More theme-compatible
    action_box->pack_start(*install_button, false, false);

    package_box->pack_start(*action_box, false, false);

    // Add the box to the frame and return the frame
    package_frame->add(*package_box);
    return package_frame;
}


void
PackageTab::on_package_click(const std::string &package_name)
{
    m_logger->log(Logger::Info, "Clicked on package: {}", package_name.c_str());
    // This would be implemented to show package details
}