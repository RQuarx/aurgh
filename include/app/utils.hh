#pragma once
#include <expected>
#include <string>

#include <curl/curl.h>

namespace Glib { template <typename T_CppObject> class RefPtr; }
namespace Gtk { class Builder; }


namespace app
{
    auto get_builder(const std::string &p_file_path)
        -> Glib::RefPtr<Gtk::Builder>;


    auto write_callback(void        *p_contents,
                        std::size_t  p_size,
                        std::size_t  p_nmemb,
                        std::string &p_userp) -> std::size_t;


    auto perform_curl(const char *p_url)
        -> std::expected<std::string, CURLcode>;
}
