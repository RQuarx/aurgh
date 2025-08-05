#include <filesystem>

#include <gtkmm/builder.h>
#include <json/reader.h>
#include <json/value.h>

#include "app/utils.hh"
#include "config.hh"
#include "utils.hh"


namespace app
{
    auto
    get_app_file( const std::string &p_file_path ) -> std::string
    {
        for (std::filesystem::path base : UI_PATHS) {
            if (std::filesystem::exists( base / p_file_path ))
                return (base / p_file_path);

            if (std::filesystem::exists( base / "ui" / p_file_path ))
                return (base / "ui" / p_file_path);
        }

        return "";
    }


    auto
    get_builder( const std::string &p_file_path
                ) -> std::expected<builder, std::string>
    {
        const std::string ui_file { get_app_file(p_file_path) };
        if (ui_file.empty()) {
            return utils::unexpected("App file {} does not exist", p_file_path);
        }

        Glib::RefPtr<Gtk::Builder> builder;
        try {
            builder = Gtk::Builder::create_from_file(ui_file);
        } catch (const Glib::Error &e) {
            return utils::unexpected(e.what().raw());
        }

        return builder;
    }


    auto
    write_callback( void        *p_contents,
                    size_t       p_size,
                    size_t       p_nmemb,
                    std::string &p_userp ) -> size_t
    {
        size_t total_size { p_size * p_nmemb };
        p_userp.append(static_cast<char *>(p_contents), total_size);
        return total_size;
    }


    auto
    perform_curl( const char *p_url ) -> std::expected<std::string, CURLcode>
    {
        CURL *curl { curl_easy_init() };

        std::string buff;
        curl_easy_setopt(curl, CURLOPT_URL, p_url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buff);
        CURLcode retval = curl_easy_perform(curl);

        curl_easy_cleanup(curl);

        if (retval != CURLE_OK) return utils::unexpected(retval);
        return buff;
    }
}


namespace Json
{
    auto
    from_string( const std::string &p_str
               ) -> std::expected<Json::Value, std::string>
    {
        std::istringstream iss { p_str };
        Json::Value root;

        try {
            iss >> root;
        } catch (const std::exception &e) {
            return utils::unexpected(p_str);
        }
        return root;
    }
}