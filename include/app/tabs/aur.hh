#pragma once
#include <queue>

#include <glibmm/dispatcher.h>
#include <gtkmm/box.h>

#include "app/package.hh"
#include "app/tabs/card.hh"
#include "app/types.hh"

namespace Gtk
{
    class Builder;
    class FlowBox;
}
namespace Json { class Value; }


namespace app::aur
{
    class Tab : public Gtk::Box
    {
    public:
        Tab(BaseObjectType                   *p_cobject,
            const Glib::RefPtr<Gtk::Builder> &p_builder,
            signal_type                       p_signal);

    private:
        Gtk::Box *m_content;

        std::vector<Card>        m_cards;
        std::vector<Json::Value> m_pkgs;

        Glib::Dispatcher m_on_search_dispatch;

        std::queue<Package> m_package_queue;

    protected:
        void on_active(TabType          p_tab,
                       CriteriaWidgets &p_widgets,
                       CriteriaType     p_type);

        static auto search_pkg(const std::string &p_pkg,
                               const std::string &p_search_by) -> Json::Value;

        static auto get_pkgs_info(const Json::Value &p_pkgs) -> Json::Value;


        void add_cards_to_box();
    };
}
