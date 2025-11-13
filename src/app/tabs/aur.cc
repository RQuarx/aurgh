#include <thread>

#include <gtkmm.h>

#include "app/tabs/aur.hh"
#include "app/utils.hh"
#include "config.hh"
#include "log.hh"
#include "utils.hh"

using app::aur::Tab;

namespace
{
    void
    sort_json(std::vector<Json::Value> &p_jsons,
              const std::string        &p_sort_by,
              bool                      p_reverse)
    {
        std::ranges::sort(
            p_jsons,
            [&p_sort_by, &p_reverse](const auto &a, const auto &b) -> bool
            {
                const Json::Value &a_val { a[p_sort_by] };
                const Json::Value &b_val { b[p_sort_by] };

                if (a_val.isInt())
                {
                    return p_reverse ? a_val.asInt() > b_val.asInt()
                                     : a_val.asInt() < b_val.asInt();
                }
                return p_reverse ? a_val.asString() > b_val.asString()
                                 : a_val.asString() < b_val.asString();
            });
    }
}


Tab::Tab(BaseObjectType *p_object, const Glib::RefPtr<Gtk::Builder> &p_builder)
    : BaseTab(p_object, p_builder)
{
    this->on_search_dispatcher.connect(
        sigc::mem_fun(*this, &Tab::add_cards_to_box));
}


void
Tab::activate(CriteriaWidgets &p_criteria, CriteriaType p_type)
{
    Gtk::Box *content_box { get_content_box() };

    logger.log<DEBUG>("AUR tab activated");

    if (p_type == CriteriaType::SEARCH_TEXT)
    {
        this->cards.clear();
        this->pkgs.clear();
        content_box->foreach(
            [content_box](Gtk::Widget &p_child)
            {
                content_box->remove(p_child);
                delete &p_child;
            });

        const auto [search_by, sort_by, search_text,
                    reverse] { p_criteria.get_string() };

        /* TODO: implement a way to tell the user regarding these */
        if (search_text.empty()) return;
        if (search_by.empty()) return;

        std::jthread(
            [=, this]() -> void
            {
                Json::Value result { Tab::search_package(search_text,
                                                         search_by) };
                Json::Value infos { Tab::get_pkgs_info(result["results"]) };

                {
                    std::scoped_lock lock { this->pkgs_mutex };

                    this->pkgs.clear();
                    this->pkgs.reserve(infos.size());
                    for (const Json::Value &pkg : infos)
                        this->pkgs.emplace_back(pkg);
                }

                sort_json(this->pkgs, sort_by.empty() ? "NumVotes" : sort_by,
                          reverse);

                this->on_search_dispatcher.emit();
            });
    }
}


void
Tab::close()
{
}


auto
Tab::search_package(const std::string &p_pkg, const std::string &p_search_by)
    -> Json::Value
{
    logger.log<INFO>("Searching for {} by {}", p_pkg, p_search_by);
    std::string url { std::format("{}/search/{}?by={}", AUR_URL, p_pkg,
                                  p_search_by) };
    auto        retval { perform_curl(url.c_str()) };
    if (!retval)
    {
        logger.log<ERROR>("Failed to search for {} by {}: {}", p_pkg,
                          p_search_by, curl_easy_strerror(retval.error()));
        return Json::nullValue;
    }

    try
    {
        return Json::from_string(*retval);
    }
    catch (const std::exception &e)
    {
        logger.log<ERROR>("Malformed value returned from the "
                          "AUR: {}, output: {}",
                          e.what(), *retval);
        return Json::nullValue;
    }
}


auto
Tab::get_pkgs_info(const Json::Value &p_pkgs) -> Json::Value
{
    logger.log<INFO>("Fetching information for {} packages.", p_pkgs.size());
    std::string url { std::format("{}/info?", AUR_URL) };

    for (Json::ArrayIndex i { 0 }; i < p_pkgs.size(); i++)
        url.append(std::format("arg%5B%5D={}&", p_pkgs[i]["Name"].asString()));
    url.pop_back();

    auto retval { perform_curl(url.c_str()) };
    if (!retval)
    {
        logger.log<ERROR>("Failed to get informations for {} packages: {}",
                          p_pkgs.size(), curl_easy_strerror(retval.error()));
        return Json::nullValue;
    }

    auto json { Json::from_string(*retval) };

    return json["results"];
}


void
Tab::add_cards_to_box()
{
    std::vector<Json::Value> pkgs_copy;

    {
        std::scoped_lock lock { this->pkgs_mutex };
        pkgs_copy = this->pkgs;
    }

    for (const auto &pkg : pkgs_copy)
    {
        this->cards.emplace_back(pkg, Card::INSTALL);

        if (this->cards.back().is_valid())
        {
            this->get_content_box()->pack_start(
                *this->cards.back().get_widget());
            continue;
        }

        logger.log<WARN>("Invalid card for {}", pkg["Name"].asString());
    }
    this->get_content_box()->show_all_children();
}
