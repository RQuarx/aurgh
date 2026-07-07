#pragma once
#include <chrono>
#include <vector>

#include <glibmm/ustring.h>


namespace aurgh
{
    struct package
    {
        Glib::ustring name;
        std::string   version;
        Glib::ustring description;
        std::string   repo;
    };


    struct package_details
    {
        std::vector<std::string> licenses;
        std::vector<std::string> depends;
        std::vector<std::string> make_depends;
        std::vector<std::string> opt_depends;
        std::string              url;
        std::chrono::seconds last_updated;
    };
}
