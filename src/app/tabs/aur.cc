#include <thread>
#include <utility>

#include <gtkmm.h>
#include <json/value.h>

#include "app/dialog.hh"
#include "app/tabs/aur.hh"
#include "app/tabs/card.hh"
#include "app/utils.hh"
#include "config.hh"
#include "logger.hh"
#include "utils.hh"

using app::aur::Tab;

#define DOMAIN "app::tab::aur"


namespace
{
    void
    sort_json(std::vector<Json::Value> &jsons,
              const std::string        &sort_by,
              bool                      reverse)
    {
        std::ranges::sort(
            jsons,
            [&sort_by, &reverse](const auto &a, const auto &b) -> bool
            {
                const Json::Value &a_val { a[sort_by] };
                const Json::Value &b_val { b[sort_by] };

                if (a_val.isInt())
                    return reverse ? a_val.asInt() < b_val.asInt()
                                   : a_val.asInt() > b_val.asInt();

                return reverse ? a_val.asString() > b_val.asString()
                               : a_val.asString() < b_val.asString();
            });
    }
}


Tab::Tab(BaseObjectType *object, const Glib::RefPtr<Gtk::Builder> &builder)
    : app::Tab(object, builder, "AUR", DOMAIN)
{
    m_add_cards_dispatcher.connect(
        sigc::mem_fun(*this, &Tab::add_cards_to_box));
}


void
Tab::activate(CriteriaWidgets &criteria, CriteriaType type)
{
    logger[Level::TRACE, DOMAIN]("AUR tab activated");

    if (type == CriteriaType::SEARCH_TEXT)
    {
        const auto &[search_by, sort_by, search_text,
                     reverse] { criteria.get_values() };

        if (search_text.empty() || search_by.empty())
        {
            ChoiceDialog::show_error(
                this,
                "Required search criteria (search entry or search "
                "by) is empty",
                { "OK" });

            return;
        }

        m_pkgs.clear();
        clear_content_box();

        std::jthread {
            [=, this]() -> void
            { search_and_fill(search_by, sort_by, search_text, reverse); }
        }.detach();
        return;
    }

    if (type == CriteriaType::NONE)
    {
        if (!m_pkgs.empty() && m_cards.empty()) this->add_cards_to_box();
        return;
    }

    clear_content_box();

    const auto &[search_by, sort_by, search_text,
                 reverse] { criteria.get_values() };

    std::jthread { &Tab::reload_content, this, sort_by, reverse }.detach();
}


void
Tab::close()
{
    if (m_cards.empty()) return;
    logger[Level::DEBUG, DOMAIN]("Clearing cards from tab {}", get_tab_name());
    clear_content_box();
}


auto
Tab::search_package(std::string_view pkg, std::string_view search_by)
    -> Json::Value
{
    logger[Level::INFO, DOMAIN]("Searching for {} by {}", pkg, search_by);
    std::string url { std::format("{}/search/{}?by={}", AUR_URL, pkg,
                                  search_by) };

    auto retval { utils::perform_curl(url.c_str()) };

    if (!retval)
    {
        std::string message { std::format("Failed to search for {} by {}: {}",
                                          pkg, search_by,
                                          curl_easy_strerror(retval.error())) };

        logger[Level::ERROR, DOMAIN]("{}", message);
        if (app::ChoiceDialog::show_error_async(this, message).get() == "Quit")
            std::exit(1);

        return Json::nullValue;
    }

    try
    {
        return Json::from_string(*retval);
    }
    catch (const std::exception &e)
    {
        logger[Level::ERROR, DOMAIN]("Malformed value returned from the "
                                     "AUR: {}, output: {}",
                                     e.what(), *retval);

        std::string result {
            ChoiceDialog::show_error_async(
                this, "Malformed value returned from AUR API, see log for more "
                      "information")
                .get()
        };

        if (result == "Quit") std::exit(retval.error());
        return Json::nullValue;
    }
}


auto
Tab::get_pkgs_info(const Json::Value &pkgs) -> Json::Value
{
    std::string url { std::format("{}/info?", AUR_URL) };

    for (Json::ArrayIndex i { 0 }; i < pkgs.size(); i++)
        url.append(std::format("arg%5B%5D={}&", pkgs[i]["Name"].asString()));
    url.pop_back();

    auto retval { utils::perform_curl(url.c_str()) };
    if (!retval)
    {
        std::string message { std::format(
            "Failed to get informations for {} packages: {}", pkgs.size(),
            curl_easy_strerror(retval.error())) };

        logger[Level::ERROR, DOMAIN]("{}", message);
        if (app::ChoiceDialog::show_error(this, message) == "Quit")
            std::exit(1);

        return Json::nullValue;
    }

    try
    {
        return Json::from_string(*retval)["results"];
    }
    catch (const std::exception &e)
    {
        std::string message { std::format("{}: {}", e.what(), url) };

        logger[Level::ERROR, DOMAIN]("{}", message);
        if (app::ChoiceDialog::show_error_async(this, message).get() == "Quit")
            std::exit(1);

        return Json::nullValue;
    }
}


void
Tab::add_cards_to_box()
{
    std::vector<Json::Value> pkgs_copy;

    {
        std::scoped_lock lock { m_pkgs_mutex };
        pkgs_copy = m_pkgs;
    }

    for (const auto &pkg : pkgs_copy)
    {
        bool in_queue { contains_pkg(pkg["Name"].asString()) };

        auto card { std::make_unique<Card>(pkg, Card::Type::INSTALL,
                                           in_queue) };

        if (!card->is_valid())
        {
            Package    &package { card->get_package() };
            std::string message { std::format("Failed to load package {}: {}",
                                              package[PKG_NAME],
                                              package.get_error_message()) };
            logger[Level::ERROR, DOMAIN]("{}", message);
            if (app::ChoiceDialog::show_error(this, message) == "Quit")
                std::exit(1);

            continue;
        }

        Card *card_ptr { card.get() };
        card->signal_on_add_to_queue().connect(
            [this, card_ptr](bool active) -> void
            {
                if (!active)
                    this->pop_pkg(card_ptr->get_package()[PKG_NAME]);
                else
                    this->push_pkg(card_ptr->get_package());
            });

        this->get_content_box()->pack_start(*card->get_widget());
        m_cards.emplace_back(std::move(card));
    }
}


void
Tab::clear_content_box()
{
    Gtk::Box *content_box { this->get_content_box() };

    m_cards.clear();
    content_box->foreach([content_box](Gtk::Widget &child) -> void
                         { content_box->remove(child); });
}


void
Tab::search_and_fill_pkgs(std::string_view search_text,
                          std::string_view search_by)
{
    Json::Value result { Tab::search_package(search_text, search_by) };
    if (result == Json::nullValue) return;

    Json::Value infos { Tab::get_pkgs_info(result["results"]) };
    if (infos == Json::nullValue) return;
}


void
Tab::reload_content(const std::string &sort_by, bool reverse)
{
    sort_json(m_pkgs, sort_by.empty() ? "NumVotes" : sort_by, reverse);
    m_add_cards_dispatcher.emit();
}


void
Tab::search_and_fill(std::string_view   search_by,
                     const std::string &sort_by,
                     std::string_view   search_text,
                     bool               reverse)
{
    Json::Value results { Tab::search_package(search_text, search_by) };
    if (results == Json::nullValue) return;
    results = results["results"];

    logger[Level::DEBUG, DOMAIN]("Getting information about {} packages",
                                 results.size());

    if (results.size() > 100)
    {
        Json::Value all_info { Json::arrayValue };
        Json::Value chunk { Json::arrayValue };

        for (Json::ArrayIndex start { 0 }; start < results.size(); start += 100)
        {
            chunk.clear();

            auto end { std::min<Json::ArrayIndex>(start + 100,
                                                  results.size()) };
            for (Json::ArrayIndex i { start }; i < end; i++)
                chunk.append(results[i]);

            Json::Value info { get_pkgs_info(chunk) };
            if (info == Json::nullValue) return;

            for (const auto &item : info)
            {
                if (item == Json::nullValue) return;
                m_pkgs.emplace_back(item);
            }

            reload_content(sort_by, reverse);
        }
    }
    else
    {
        for (const auto &item : Tab::get_pkgs_info(results))
            m_pkgs.emplace_back(item);

        reload_content(sort_by, reverse);
    }
}
