#pragma once
#include <glibmm/dispatcher.h>
#include <json/value.h>

#include "app/tab.hh"


namespace app
{
    class ChoiceDialog;
    class Card;
}

namespace app::aur
{
    /**
     * A tab that displays packages from the Arch User Repository.
     *
     * [note]-----------------------------------------------------
     *
     * The Tab fetches package data from the AUR, populates the UI with the
     * `Card` widgets for each package, and responds to search/sort criteria
     * changes.
     */
    class Tab : public app::Tab
    {
    public:
        Tab(BaseObjectType                   *p_object,
            const Glib::RefPtr<Gtk::Builder> &p_builder);


        void activate(CriteriaWidgets &p_criteria,
                      CriteriaType     p_type) override;


        void close() override;

    private:
        std::vector<std::unique_ptr<Card>> cards;
        std::vector<Json::Value>           pkgs;
        std::mutex                         pkgs_mutex;

        Glib::Dispatcher on_search_dispatcher;

    protected:
        /**
         * Search for packages using the AUR API.
         *
         * [params]-----------------------------
         *
         * `p_pkg`:
         *   Package name or search string.
         *
         * `p_search_by`:
         *   Field to search by (e.g., "Name", "Maintainer").
         *
         * [returns]----------------------------
         *
         * JSON object containing search results or `Json::nullValue` on failure.
         */
        auto search_package(std::string_view p_pkg,
                            std::string_view p_search_by) -> Json::Value;


        /**
         * Retrieve detailed information for a list of packages.
         *
         * [params]---------------------------------------------
         *
         * `p_pkgs`:
         *   JSON array of package names.
         *
         * [returns]--------------------------------------------
         *
         * JSON array containing detailed package info,
         * or `Json::nullValue` on failure.
         */
        auto get_pkgs_info(const Json::Value &p_pkgs) -> Json::Value;


        /* Convert `pkgs` into `Card` widgets and add them to the content box. */
        void add_cards_to_box();


        /* Remove all cards from the content box and
           clear the internal `Card`s vector. */
        void clear_content_box();


        /**
         * Perform a search using the given criteria and populate `pkgs`.
         *
         * [params]--------------------------------------------------
         *
         * `p_search_text`:
         *   Text to search for.
         *
         * `p_search_by`:
         *   Field to search by.
         */
        void search_and_fill_pkgs(std::string_view p_search_text,
                                  std::string_view p_search_by);


        /**
         * Sort the packages by a given field and update the UI.
         *
         * [params]-----------------------------------------------------------
         *
         * `p_sort_by`:
         *   Field to sort packages by (e.g., "NumVotes").
         *
         * `p_reverse`:
         *   If true, sort in descending order.
         */
        void reload_content(const std::string &p_sort_by, bool p_reverse);
    };
}
