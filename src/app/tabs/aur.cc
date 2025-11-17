#include <thread>
#include <utility>

#include <gtkmm.h>
#include <json/value.h>

#include "app/dialog.hh"
#include "app/tabs/aur.hh"
#include "app/tabs/card.hh"
#include "app/utils.hh"
#include "config.hh"
#include "log.hh"
#include "utils.hh"

using enum LogLevel;
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
                    return p_reverse ? a_val.asInt() < b_val.asInt()
                                     : a_val.asInt() > b_val.asInt();

                return p_reverse ? a_val.asString() > b_val.asString()
                                 : a_val.asString() < b_val.asString();
            });
    }


    void
    log_and_show(Gtk::Widget       *p_widget,
                 const std::string &p_message,
                 bool               p_async = false)
    {
        logger.log<ERROR>("{}", p_message);

        std::string result {
            p_async
                ? app::ChoiceDialog::show_error_async(p_widget, p_message).get()
                : app::ChoiceDialog::show_error(p_widget, p_message)
        };

        if (result == "Quit") std::exit(1);
    }


    auto
    handle_json_error(Gtk::Widget       *p_widget,
                      const std::string &p_message,
                      bool               p_async = false)
    {
        log_and_show(p_widget, p_message, p_async);
        return Json::nullValue;
    }
}


Tab::Tab(BaseObjectType *p_object, const Glib::RefPtr<Gtk::Builder> &p_builder)
    : app::Tab(p_object, p_builder)
{
    this->set_name("AUR");

    this->on_search_dispatcher.connect(
        sigc::mem_fun(*this, &Tab::add_cards_to_box));
}


void
Tab::activate(CriteriaWidgets &p_criteria, CriteriaType p_type)
{
    logger.log<DEBUG>("AUR tab activated");

    if (p_type == CriteriaType::SEARCH_TEXT)
    {
        const auto &[search_by, sort_by, search_text,
                     reverse] { p_criteria.get_values() };

        if (search_text.empty() || search_by.empty())
        {
            ChoiceDialog::show_error(
                this,
                "Required search criteria (search entry or search "
                "by) is empty",
                { "OK" });

            return;
        }

        this->pkgs.clear();
        clear_content_box();

        std::jthread {
            [=, this]() -> void
            {
                search_and_fill_pkgs(search_text, search_by);
                reload_content(sort_by, reverse);
            }
        }.detach();
        return;
    }

    if (p_type == CriteriaType::NONE)
    {
        if (!this->pkgs.empty() && this->cards.empty())
            this->add_cards_to_box();
        return;
    }

    clear_content_box();

    const auto &[search_by, sort_by, search_text,
                 reverse] { p_criteria.get_values() };

    std::jthread { &Tab::reload_content, this, sort_by, reverse }.detach();
}


void
Tab::close()
{
    if (this->cards.empty()) return;

    logger.log<DEBUG>("Clearing cards from tab {}", this->get_name());

    clear_content_box();
}


auto
Tab::search_package(std::string_view p_pkg, std::string_view p_search_by)
    -> Json::Value
{
    logger.log<INFO>("Searching for {} by {}", p_pkg, p_search_by);
    std::string url { std::format("{}/search/{}?by={}", AUR_URL, p_pkg,
                                  p_search_by) };

    auto retval { utils::perform_curl(url.c_str()) };

    if (!retval)
    {
        std::string message { std::format("Failed to search for {} by {}: {}",
                                          p_pkg, p_search_by,
                                          curl_easy_strerror(retval.error())) };
        return handle_json_error(this, message, true);
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
Tab::get_pkgs_info(const Json::Value &p_pkgs) -> Json::Value
{
    logger.log<INFO>("Fetching information for {} packages", p_pkgs.size());
    std::string url { std::format("{}/info?", AUR_URL) };

    for (Json::ArrayIndex i { 0 }; i < p_pkgs.size(); i++)
        url.append(std::format("arg%5B%5D={}&", p_pkgs[i]["Name"].asString()));
    url.pop_back();

    auto retval { utils::perform_curl(url.c_str()) };
    if (!retval)
    {
        std::string message { std::format(
            "Failed to get informations for {} packages: {}", p_pkgs.size(),
            curl_easy_strerror(retval.error())) };

        return handle_json_error(this, message);
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
        bool in_queue { contains_pkg(pkg["Name"].asString()) };

        auto card { std::make_unique<Card>(pkg, Card::Type::INSTALL,
                                           in_queue) };

        if (!card->is_valid())
        {
            Package    &package { card->get_package() };
            std::string message { std::format("Failed to load package {}: {}",
                                              package[PKG_NAME],
                                              package.get_error_message()) };
            log_and_show(this, message);
            continue;
        }

        Card *card_ptr { card.get() };
        card->signal_on_add_to_queue().connect(
            [this, card_ptr](bool p_active) -> void
            {
                if (!p_active)
                    this->pop_pkg(card_ptr->get_package()[PKG_NAME]);
                else
                    this->push_pkg(card_ptr->get_package());
            });

        this->get_content_box()->pack_start(*card->get_widget());
        this->cards.emplace_back(std::move(card));
    }
}


void
Tab::clear_content_box()
{
    Gtk::Box *content_box { this->get_content_box() };

    this->cards.clear();
    content_box->foreach([content_box](Gtk::Widget &p_child) -> void
                         { content_box->remove(p_child); });
}


void
Tab::search_and_fill_pkgs(std::string_view p_search_text,
                          std::string_view p_search_by)
{
    Json::Value result { Tab::search_package(p_search_text, p_search_by) };
    if (result == Json::nullValue) return;

    Json::Value infos { Tab::get_pkgs_info(result["results"]) };
    if (infos == Json::nullValue) return;

    {
        std::scoped_lock lock { this->pkgs_mutex };

        this->pkgs.clear();
        this->pkgs.reserve(infos.size());
        for (const Json::Value &pkg : infos) this->pkgs.emplace_back(pkg);
    }
}


void
Tab::reload_content(const std::string &p_sort_by, bool p_reverse)
{
    sort_json(this->pkgs, p_sort_by.empty() ? "NumVotes" : p_sort_by,
              p_reverse);
    this->on_search_dispatcher.emit();
}
