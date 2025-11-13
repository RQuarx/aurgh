#include <gtkmm/builder.h>

#include "app/utils.hh"
#include "log.hh"
#include "utils.hh"


namespace app
{
    auto
    get_builder(const std::string &p_file_path) -> Glib::RefPtr<Gtk::Builder>
    {
        try
        {
            return Gtk::Builder::create_from_resource(p_file_path);
        }
        catch (const Glib::Error &e)
        {
            logger.log<ERROR>("Failed to get builder file {}: {}",
                              p_file_path, e.what().raw());
            std::terminate();
        }
    }


    auto
    write_callback(void        *p_contents,
                   size_t       p_size,
                   size_t       p_nmemb,
                   std::string &p_userp) -> size_t
    {
        size_t total_size { p_size * p_nmemb };
        p_userp.append(static_cast<char *>(p_contents), total_size);
        return total_size;
    }


    auto
    perform_curl(const char *p_url) -> std::expected<std::string, CURLcode>
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
