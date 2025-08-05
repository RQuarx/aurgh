#pragma once
#include <expected>
#include <string>
#include <curl/curl.h>

namespace Glib
{
    template<typename T_CppObject>
    class RefPtr;
}
namespace Gtk { class Builder; }
namespace Json { class Value; }

using builder = Glib::RefPtr<Gtk::Builder>;


namespace app
{
    /**
     * @brief Searches for a path inside a list of paths UI_PATHS.
     *
     * @param p_file_path The top-level file path to search inside UI_PATHS.
     * @return An std::string object containing the path to the valid path,
     *         or an empty std::string object if no valid path were found.
     *
     * The function searches for @p p_file_path inside UI_PATHS,
     * it first checks if {base}/p_file_path is valid, then it checks for
     * {base}/ui/p_file_path. Where base is the base path defined inside UI_PATHS.
     *
     * If none of the check paths exist,
     * then the function will return an empty string.
     */
    auto get_app_file( const std::string &p_file_path ) -> std::string;


    /**
     * @brief A helper function for getting a Glib::RefPtr<Gtk::Builder> object.
     *
     * @param p_file_path The top-level file path pointing to a .xml builder file.
     * @return An std::expected<builder, std::string> object,
     *         where the Err string will contain the error message supplied by
     *         Glib::Error::what()::raw().
     *
     * @note This function is a wrapper for get_app_file.
     */
    auto get_builder( const std::string &p_file_path
                    ) -> std::expected<builder, std::string>;


    /** @brief A write callback function for libCURL. */
    auto write_callback( void       *p_contents,
                        size_t       p_size,
                        size_t       p_nmemb,
                        std::string &p_userp ) -> size_t;


    /**
     * @brief A helper function to perform a GET request to @p url .
     *
     * @param p_url The url to perform the GET request to.
     * @return An std::expected<std::string, CURLcode> object, where the expected
     *         std::string object would contain the result of the GET request.
     */
    auto perform_curl( const char *p_url
                     ) -> std::expected<std::string, CURLcode>;
}


namespace Json
{
    /**
     * @brief A wrapper for converting std::string object to a Json::Value object.
     *
     * @return An std::expected<Json::Value, std::string> object, where the Err
     *         std::string object contains the error message from an exception.
     */
    auto from_string( const std::string &p_str
                    ) -> std::expected<Json::Value, std::string>;
}