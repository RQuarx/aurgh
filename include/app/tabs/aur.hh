#pragma once
#include "app/base_tab.hh"

#include <glibmm/dispatcher.h>


namespace app { class Card; }

namespace app::aur
{
    class Tab : public BaseTab
    {
    public:
        Tab(BaseObjectType                   *p_object,
            const Glib::RefPtr<Gtk::Builder> &p_builder);


        /**
         * @brief Activate the tab.
         *
         * @param criteria A @a CriteriaWidgets struct.
         * @param type     The type of the criteria that changed.
         *
         * Loads the package cards contained within the tab.
         */
        void activate(CriteriaWidgets &p_criteria,
                      CriteriaType     p_type) override;


        /**
         * @brief Closes the tab.
         */
        void close() override;

    private:
        std::vector<Card>        cards;
        std::vector<Json::Value> pkgs;
        std::mutex               pkgs_mutex;

        Glib::Dispatcher on_search_dispatcher;

    protected:
        static auto search_package(std::string_view p_pkg,
                                   std::string_view p_search_by) -> Json::Value;


        static auto get_pkgs_info(const Json::Value &p_pkgs) -> Json::Value;


        void add_cards_to_box();


        void clear_content_box();
        void search_and_fill(std::string_view p_search_text,
                             std::string_view p_search_by);
        void reload_content(const std::string &p_sort_by, bool p_reverse);
    };
}
