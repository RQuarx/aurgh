#pragma once
#include <map>

#include <gtkmm/box.h>

#include "package.hh"

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
        Gtk::SearchEntry  *search_entry;
        Gtk::CheckButton  *reverse;


        /**
         * Retrieve the current criteria values as simple types.
         *
         * [note]-----------------------------------------------
         * Returns a tuple containing a
         *   - search-by ID string
         *   - sort-by ID string
         *   - search text
         *   - reverse flag
         *
         * Each value is extracted directly from the associated widget.
         */
        [[nodiscard]]
        auto get_values() const
            -> std::tuple<std::string, std::string, std::string, bool>;
    };


    /* An enum containing the types of search/sort criteria. */
    enum class CriteriaType : std::uint8_t
    {
        SEARCH_BY,   /* `CriteriaWidgets::search_by`    has changed. */
        SORT_BY,     /* `CriteriaWidgets::sort_by`      has changed. */
        SEARCH_TEXT, /* `CriteriaWidgets::search_entry` has changed. */
        REVERSE,     /* `CriteriaWidgets::reverse`      has changed. */
        NONE         /* Nothing has changed with the criteria.       */
    };


    enum class TabType : std::uint8_t
    {
        AUR,
        MAIN,
        INSTALLED,
        NONE
    };


    /* A base class for tabs shown in the UI. */
    class Tab : public Gtk::Box
    {
    public:
        using signal_signature_queue = sigc::signal<void(
            const std::string &, std::map<std::string, Package> &)>;


        Tab(BaseObjectType                   *p_object,
            const Glib::RefPtr<Gtk::Builder> &p_builder);


        /**
         * Called when the tab is shown to the user, or if a criteria changed.
         *
         * [params]-----------------------------------------------------------
         *
         * `p_type`:
         *   Contains the type of criteria that changed. If it is
         *   `CriteriaType::NONE`, the tab is activated because it has been
         *   shown to the user.
         */
        virtual void activate(CriteriaWidgets &p_criteria, CriteriaType p_type)
            = 0;


        /* Called when the tab is closed. */
        virtual void close() = 0;


        /* Sets the name of the tab. */
        auto set_name(std::string p_tab_name) -> Tab &;


        /* Get the name of the tab */
        [[nodiscard]]
        auto get_name() const -> std::string;


        /**
         * Get a `sigc::signal` connected to the internal package queue.
         *
         * [note]-------------------------------------------------------
         *
         * The signal is emitted when the queue is mutated (pushed/popped).
        */
        [[nodiscard]]
        auto signal_queue_update() -> signal_signature_queue;

    private:
        Gtk::Box *m_content_box;

        std::string                    m_tab_name;
        std::map<std::string, Package> m_package_queue;

        signal_signature_queue m_queue_signal;

    protected:
        /* Push a package to the internal queue, and emit the queue signal. */
        void push_pkg(Package p_pkg);


        /* Pop a package from the internal queue, and emit the queue signal. */
        void pop_pkg(const std::string &p_name);


        /* Checks whether a package exist in thee internal queue. */
        auto contains_pkg(const std::string &p_name) const -> bool;


        /* Get the tab's content box. */
        [[nodiscard]]
        auto get_content_box() -> Gtk::Box *;
    };
}
