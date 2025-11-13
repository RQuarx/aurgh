#pragma once
#include <queue>

#include <glibmm/dispatcher.h>
#include <gtkmm/box.h>

#include "app/package.hh"
#include "app/tabs/card.hh"

namespace Gtk
{
    class ComboBoxText;
    class CheckButton;
    class SearchEntry;
    class Builder;
}


namespace app
{
    struct CriteriaWidgets
    {
        Gtk::ComboBoxText *search_by;
        Gtk::ComboBoxText *sort_by;
        Gtk::CheckButton  *reverse;
        Gtk::SearchEntry  *search_entry;


        /**
         * @brief Returns the widgets data in a form of strings and bool.
         *
         * The tuple contains the search-by id text, sort-by id text,
         * search-entry text and reverse check in order.
         */
        [[nodiscard]]
        auto get_string(this CriteriaWidgets &self)
            -> std::tuple<std::string, std::string, std::string, bool>;
    };

    enum class CriteriaType : std::uint8_t
    {
        SEARCH_BY,
        SORT_BY,
        SEARCH_TEXT,
        REVERSE,
        NONE
    };

    enum class TabType : std::uint8_t
    {
        AUR,
        MAIN,
        INSTALLED,
        NONE
    };


    class BaseTab : public Gtk::Box
    {
    public:
        using signal_signature_queue
            = sigc::signal<void(const std::queue<Package> &)>;


        BaseTab(BaseObjectType                   *p_object,
                const Glib::RefPtr<Gtk::Builder> &p_builder);


        virtual void activate(CriteriaWidgets &p_criteria, CriteriaType p_type)
            = 0;


        virtual void close() = 0;


        auto set_name(this BaseTab &self, std::string &&p_tab_name)
            -> BaseTab &;


        [[nodiscard]]
        auto get_name(this const BaseTab &self) -> std::string;


        [[nodiscard]]
        auto signal_queue_update() -> signal_signature_queue;

    private:
        Gtk::Box           *content_box;
        std::string         tab_name;
        std::queue<Package> package_queue;

        signal_signature_queue queue_signal;

    protected:
        virtual void push_pkg(Package &&p_pkg);
        virtual void pop_pkg();


        [[nodiscard]]
        auto get_content_box(this BaseTab &self) -> Gtk::Box *;
    };
}
