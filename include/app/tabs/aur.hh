#pragma once
#include <memory>
#include <gtkmm/box.h>
#include "app/types.hh"

namespace Gtk { class Builder; class FlowBox; }
namespace Json { class Value; }
class Logger;
class Card;


namespace aur
{
    class Tab : public Gtk::Box
    {
    public:
        Tab( BaseObjectType                   *cobject,
             const Glib::RefPtr<Gtk::Builder> &builder,
             const std::shared_ptr<Logger>    &logger,
             signal_type                       signal);

    private:
        std::shared_ptr<Logger> m_logger;

        Gtk::FlowBox *m_content;

        std::vector<Card> m_cards;

    protected:
        void on_active( TabType          tab,
                        CriteriaWidgets &widgets,
                        CriteriaType     type );

        auto search_pkg( const std::string &pkg,
                         const std::string &search_by ) -> Json::Value;

        auto get_pkgs_info(
            const std::vector<std::string> &pkgs ) -> Json::Value;

        auto get_pkgs_info(
            const std::vector<std::reference_wrapper<const Json::Value>> &pkgs
        ) -> Json::Value;
    };
}