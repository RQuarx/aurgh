#pragma once
#include <string>

#include <curl/curl.h>

namespace Glib { template <typename T_CppObject> class RefPtr; }
namespace Gtk
{
    class Builder;
    class Window;
    class Widget;
}


namespace app
{
    /* Get a `Gtk::Builder` without the typing. */
    auto get_builder(const std::string &file_path)
        -> Glib::RefPtr<Gtk::Builder>;
}
