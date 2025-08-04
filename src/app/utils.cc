#include <filesystem>

#include <gtkmm/builder.h>
#include <json/reader.h>
#include <json/value.h>

#include "app/utils.hh"
#include "config.hh"
#include "error.hh"


auto
get_app_file( const std::string &file_path ) -> std::string
{
    for (std::filesystem::path base : UI_PATHS) {
        if (std::filesystem::exists( base / file_path ))
            return (base / file_path);

        if (std::filesystem::exists( base / "ui" / file_path ))
            return (base / "ui" / file_path);
    }

    return "";
}


auto
get_builder(const std::string &file_path ) -> std::expected<builder, std::string>
{
    const std::string ui_file { get_app_file(file_path) };
    if (ui_file.empty()) {
        return err::unexpected("App file {} does not exist", file_path);
    }

    Glib::RefPtr<Gtk::Builder> builder;
    try {
        builder = Gtk::Builder::create_from_file(ui_file);
    } catch (const Glib::Error &e) {
        return err::unexpected(e.what().raw());
    }

    return builder;
}


auto
write_callback( void        *contents,
                size_t       size,
                size_t       nmemb,
                std::string &userp ) -> size_t
{
    size_t total_size { size * nmemb };
    userp.append(static_cast<char *>(contents), total_size);
    return total_size;
}


auto
perform_curl( const char *url ) -> std::expected<std::string, CURLcode>
{
    CURL *curl { curl_easy_init() };

    std::string buff;
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buff);
    CURLcode retval = curl_easy_perform(curl);

    curl_easy_cleanup(curl);

    if (retval != CURLE_OK) return std::unexpected(retval);
    return buff;
}


auto
json_from_string(const std::string &str ) -> std::expected<Json::Value, std::string>
{
    std::istringstream iss { str };
    Json::Value root;

    try {
        iss >> root;
    } catch (const std::exception &e) {
        return err::unexpected(str);
    }
    return root;
}