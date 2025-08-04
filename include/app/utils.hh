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


/**
 * @brief Searches for a path inside a list of paths UI_PATHS.
 *
 * @param file_path The top-level file path to search inside UI_PATHS.
 * @return An std::string object containing the path to the valid path,
 *         or an empty std::string object if no valid path were found.
 *
 * The function searches for @p file_path inside UI_PATHS,
 * it first checks if {base}/file_path is valid, then it checks for
 * {base}/ui/file_path. Where base is the base path defined inside UI_PATHS.
 *
 * If none of the check paths exist,
 * then the function will return an empty string.
 */
auto get_app_file( const std::string &file_path ) -> std::string;


/**
 * @brief A helper function for getting a Glib::RefPtr<Gtk::Builder> object.
 *
 * @param file_path The top-level file path pointing to a .xml builder file.
 * @return An std::expected<builder, std::string> object,
 *         where the Err string will contain the error message supplied by
 *         Glib::Error::what()::raw().
 *
 * @note This function is a wrapper for get_app_file.
 */
auto get_builder( const std::string &file_path ) -> std::expected<builder, std::string>;


/** @brief A write callback function for libCURL. */
auto write_callback( void        *contents,
                     size_t       size,
                     size_t       nmemb,
                     std::string &userp ) -> size_t;


/**
 * @brief A helper function to perform a GET request to @p url .
 *
 * @param url The url to perform the GET request to.
 * @return An std::expected<std::string, CURLcode> object, where the expected
 *         std::string object would contain the result of the GET request.
 */
auto perform_curl( const char *url ) -> std::expected<std::string, CURLcode>;


/**
 * @brief A wrapper for converting std::string object to a Json::Value object.
 *
 * @return An std::expected<Json::Value, std::string> object, where the Err
 *         std::string object contains the error message from an exception.
 */
auto json_from_string( const std::string &str ) -> std::expected<Json::Value, std::string>;
