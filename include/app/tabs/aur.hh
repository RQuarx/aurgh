#pragma once
#include "app/base_tab.hh"


namespace app::aur
{
    class Tab : public BaseTab
    {
    public:
        Tab(BaseObjectType                   *p_object,
            const Glib::RefPtr<Gtk::Builder> &p_builder);


        void activate(CriteriaWidgets &p_criteria,
                      CriteriaType     p_type) override;


        void close() override;

    private:
        std::vector<Card>        cards;
        std::vector<Json::Value> pkgs;
        std::mutex               pkgs_mutex;

        Glib::Dispatcher on_search_dispatcher;

        int closed_counter;

    protected:
        static auto search_package(const std::string &p_pkg,
                                   const std::string &p_search_by)
            -> Json::Value;

        static auto get_pkgs_info(const Json::Value &p_pkgs) -> Json::Value;


        void add_cards_to_box();
    };
}
