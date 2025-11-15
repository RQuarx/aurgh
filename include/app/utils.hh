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
    auto get_builder(const std::string &p_file_path)
        -> Glib::RefPtr<Gtk::Builder>;


    [[nodiscard]]
    auto get_toplevel(Gtk::Widget *p_obj) -> Gtk::Window *;
}
