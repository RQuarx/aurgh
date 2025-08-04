#include <gtkmm/comboboxtext.h>
#include <gtkmm/searchentry.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/linkbutton.h>
#include <gtkmm/builder.h>
#include <gtkmm/flowbox.h>
#include <gtkmm/label.h>
#include <curl/curl.h>
#include <json/json.h>
#include <thread>

#include "app/tabs/card.hh"
#include "app/tabs/aur.hh"
#include "app/utils.hh"
#include "aliases.hh"
#include "config.hh"
#include "log.hh"

using aur::Tab;


namespace
{
    auto
    sort_json( const Json::Value &root,
               const std::string &sort_by,
               bool               reverse
    ) -> std::vector<std::reference_wrapper<const Json::Value>>
    {
        if (!root.isArray()) return {};

        std::vector<std::reference_wrapper<const Json::Value>> result;
        result.reserve(root.size());

        for (const Json::Value &j : root)
            result.emplace_back(j);

        std::ranges::sort(result,
        [&sort_by, &reverse]( const auto &a, const auto &b ) -> bool
        {
            const Json::Value &a_val { a.get()[sort_by] };
            const Json::Value &b_val { b.get()[sort_by] };

            if (a_val.isInt()) {
                return reverse
                    ? a_val.asInt() > b_val.asInt()
                    : a_val.asInt() < b_val.asInt();
            }
            return reverse
                ? a_val.asString() > b_val.asString()
                : a_val.asString() < b_val.asString();
        });

        return result;
    }
}


Tab::Tab( BaseObjectType                *cobject,
          const builder_t               &builder,
          const std::shared_ptr<Logger> &logger,
          signal_type                    signal ) :
    Gtk::Box(cobject),
    m_logger(logger)
{
    builder->get_widget("content", m_content);

    signal.connect(sigc::mem_fun(*this, &aur::Tab::on_active));
    m_on_search_dispatch.connect(sigc::mem_fun(
        *this, &Tab::add_cards_to_box));
}


void
Tab::on_active( TabType          tab,
                CriteriaWidgets &widgets,
                CriteriaType     type )
{
    if (tab != AUR) return;

    /* Its a search signal. */
    if (type == SEARCH_ENTRY) {
        m_cards.clear();
        m_pkgs.clear();
        m_content->foreach([this]( Gtk::Widget &p_child ){
            m_content->remove(p_child);
            delete &p_child;
        });

        const std::string pkg_name  { widgets.search_entry->get_text() };
        const std::string search_by { widgets.search_by->get_active_id() };
        const std::string sort_by   { widgets.sort_by->get_active_id() };
        const bool reverse { widgets.reverse->get_active() };

        if (pkg_name.empty()) return;
        if (search_by.empty()) return;
        if (sort_by.empty()) return;

        std::jthread(
        [this, pkg_name, sort_by, search_by, reverse](){
            /* Search the package and sort the json */
            Json::Value result { search_pkg(pkg_name, search_by) };
            auto sorted { sort_json(result, sort_by, reverse) };

            for (const Json::Value &pkg : sorted)
                m_pkgs.push_back(pkg);

            m_on_search_dispatch.emit();
        }).detach();
    } else {
        /* We just need to show the previous cards generated. */
    }
}


auto
Tab::search_pkg( const std::string &pkg,
                   const std::string &search_by ) -> Json::Value
{
    std::string url { std::format("{}/search/{}?by={}", AUR_URL,
                                                        pkg, search_by) };
    m_logger->log<INFO>("Searching for {} by {}", pkg, search_by);
    auto retval { perform_curl(url.c_str()) };
    if (!retval) {
        m_logger->log<ERROR>("Failed to search for {} by {}: {}",
                              pkg, search_by,
                              curl_easy_strerror(retval.error()));
        return Json::nullValue;
    }

    Json::Value json { *json_from_string(*retval).or_else(
    [this]( const std::string &err ) -> res_or_string<Json::Value>
    {
        m_logger->log<ERROR>("Malformed input from the AUR: {}", err);
        exit(1);
        return {};
    }) };

    return json["results"];
}


auto
Tab::get_pkgs_info( const std::vector<std::string> &pkgs ) -> Json::Value
{
    m_logger->log<INFO>("Fetching information for {} packages.", pkgs.size());
    std::string url { std::format("{}/info?", AUR_URL) };

    for (const std::string &pkg : pkgs)
        url.append(std::format("arg%5B%5D={}&", pkg));
    url.pop_back();

    auto retval { perform_curl(url.c_str()) };
    if (!retval) {
        m_logger->log<ERROR>("Failed to get informations for {} packages: {}",
                              pkgs.size(), curl_easy_strerror(retval.error()));
        return Json::nullValue;
    }

    Json::Value json { *json_from_string(*retval).or_else(
    [this]( const std::string &err ) -> res_or_string<Json::Value>
    {
        m_logger->log<ERROR>("Malformed input from the AUR: {}", err);
        exit(1);
        return {};
    }) };

    return json["results"];
}


auto
Tab::get_pkgs_info(
    const std::vector<std::reference_wrapper<const Json::Value>> &pkgs
) -> Json::Value
{
    m_logger->log<INFO>("Fetching information for {} packages.", pkgs.size());
    std::string url { std::format("{}/info?", AUR_URL) };

    for (std::reference_wrapper<const Json::Value> pkg : pkgs)
        url.append(std::format("arg%5B%5D={}&", pkg.get()["Name"].asString()));
    url.pop_back();

    auto retval { perform_curl(url.c_str()) };
    if (!retval) {
        m_logger->log<ERROR>("Failed to get informations for {} packages: {}",
                              pkgs.size(), curl_easy_strerror(retval.error()));
        return Json::nullValue;
    }

    Json::Value json { *json_from_string(*retval).or_else(
    [this]( const std::string &err ) -> res_or_string<Json::Value>
    {
        m_logger->log<ERROR>("Malformed input from the AUR: {}", err);
        exit(1);
        return {};
    }) };

    return json["results"];
}


void
Tab::add_cards_to_box( void )
{
    for ( const auto &pkg : m_pkgs ) {
        m_cards.emplace_back(m_logger, pkg);

        if (!m_cards.back().is_valid()) {
            m_logger->log<WARN>("Invalid card for {}",
                                pkg["Name"].asString());
            continue;
        }

        m_content->pack_start(*m_cards.back().get_widget());
    }
    m_content->show_all_children();
}