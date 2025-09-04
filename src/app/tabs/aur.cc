#include <thread>

#include <curl/curl.h>
#include <json/json.h>
#include <gtkmm.h>

#include "app/tabs/card.hh"
#include "app/tabs/aur.hh"
#include "app/utils.hh"
#include "aliases.hh"
#include "config.hh"
#include "utils.hh"
#include "log.hh"

using app::aur::Tab;


namespace
{
    void
    sort_json( std::vector<Json::Value> &p_jsons,
               const std::string        &p_sort_by,
               bool                      p_reverse )
    {
        std::ranges::sort(p_jsons,
        [&p_sort_by, &p_reverse]( const auto &a, const auto &b ) -> bool
        {
            const Json::Value &a_val { a[p_sort_by] };
            const Json::Value &b_val { b[p_sort_by] };

            if (a_val.isInt()) {
                return p_reverse
                    ? a_val.asInt() < b_val.asInt()
                    : a_val.asInt() > b_val.asInt();
            }
            return p_reverse
                ? a_val.asString() < b_val.asString()
                : a_val.asString() > b_val.asString();
        });
    }
}


Tab::Tab( BaseObjectType                *p_cobject,
          const builder_t               &p_builder,
          const std::shared_ptr<Logger> &p_logger,
          signal_type                    p_signal ) :
    Gtk::Box(p_cobject),
    m_logger(p_logger)
{
    p_builder->get_widget("content", m_content);

    p_signal.connect(sigc::mem_fun(*this, &aur::Tab::on_active));
    m_on_search_dispatch.connect(sigc::mem_fun(
        *this, &Tab::add_cards_to_box));
}


void
Tab::on_active( TabType          p_tab,
                CriteriaWidgets &p_widgets,
                CriteriaType     p_type )
{
    if (p_tab != AUR) return;

    /* Its a search signal. */
    if (p_type == SEARCH_ENTRY) {
        m_cards.clear();
        m_pkgs.clear();
        m_content->foreach([this]( Gtk::Widget &p_child ){
            m_content->remove(p_child);
            delete &p_child;
        });

        const std::string pkg_name  { p_widgets.search_entry->get_text() };
        const std::string search_by { p_widgets.search_by->get_active_id() };
        const std::string sort_by   { p_widgets.sort_by->get_active_id() };
        const bool reverse { p_widgets.reverse->get_active() };

        if (pkg_name.empty())  return;
        if (search_by.empty()) return;

        /* Search the package and sort the json */
        std::jthread([=, this](){
            Json::Value result { search_pkg(pkg_name, search_by) };
            Json::Value infos  { get_pkgs_info(result["results"]) };

            m_pkgs.reserve(infos.size());
            for (Json::Value pkg : infos)
                m_pkgs.emplace_back(pkg);

            sort_json(m_pkgs,
                      sort_by.empty() ? "NumVotes" : sort_by,
                      reverse);

            m_on_search_dispatch.emit();
        }).detach();
    } else {
        /* We just need to show the previous cards generated. */
    }
}


auto
Tab::search_pkg( const std::string &p_pkg,
                 const std::string &p_search_by ) -> Json::Value
{
    m_logger->log<INFO>("Searching for {} by {}", p_pkg, p_search_by);
    std::string url { std::format("{}/search/{}?by={}", AUR_URL,
                                                        p_pkg, p_search_by) };
    auto retval { perform_curl(url.c_str()) };
    if (!retval) {
        m_logger->log<ERROR>("Failed to search for {} by {}: {}",
                              p_pkg, p_search_by,
                              curl_easy_strerror(retval.error()));
        return Json::nullValue;
    }

    try {
        return Json::from_string(*retval);
    } catch (const std::exception &e) {
        m_logger->log<ERROR>("Malformed value returned from the "
                             "AUR: {}, output: {}", e.what(), *retval);
        return Json::nullValue;
    }
}


auto
Tab::get_pkgs_info( const Json::Value &p_pkgs ) -> Json::Value
{
    m_logger->log<INFO>("Fetching information for {} packages.", p_pkgs.size());
    std::string url { std::format("{}/info?", AUR_URL) };

    for (Json::ArrayIndex i { 0 }; i < p_pkgs.size(); i++) {
        url.append(std::format("arg%5B%5D={}&", p_pkgs[i]["Name"].asString()));
    }
    url.pop_back();

    auto retval { perform_curl(url.c_str()) };
    if (!retval) {
        m_logger->log<ERROR>("Failed to get informations for {} packages: {}",
                              p_pkgs.size(),
                              curl_easy_strerror(retval.error()));
        return Json::nullValue;
    }

    auto json { Json::from_string(*retval) };

    return json["results"];
}


void
Tab::add_cards_to_box( void )
{
    for (const auto &pkg : m_pkgs) {
        m_cards.emplace_back(m_logger, pkg, Card::INSTALL);

        if (m_cards.back().is_valid()) {
            m_content->pack_start(*m_cards.back().get_widget());
            continue;
        }

        m_logger->log<WARN>("Invalid card for {}", pkg["Name"].asString());
    }
    m_content->show_all_children();
}