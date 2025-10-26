#pragma once
#include <expected>
#include <string>
#include <sigc++/signal.h>
#include <json/value.h>

namespace Gtk
{
    class ComboBoxText;
    class CheckButton;
    class SearchEntry;
}


namespace app
{
    struct CriteriaWidgets
    {
        Gtk::ComboBoxText *search_by;
        Gtk::ComboBoxText *sort_by;
        Gtk::CheckButton  *reverse;
        Gtk::SearchEntry  *search_entry;
    };


    enum CriteriaType : uint8_t
    {
        SEARCH_COMBO,
        SORT_COMBO,
        SEARCH_ENTRY,
        REVERSE_CHECK,
        CRITERIA_NONE
    };

    enum TabType : uint8_t
    {
        AUR,
        MAIN,
        INSTALLED,
        TAB_NONE
    };

    using signal_type = sigc::signal<void, TabType,
                                        CriteriaWidgets &,
                                        CriteriaType >;

    using void_or_err = std::expected<void,    std::string>;
    using int_or_err  = std::expected<int32_t, std::string>;
}