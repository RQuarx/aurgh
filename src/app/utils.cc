#include <glibmm.h>
#include <gtkmm/builder.h>

#include "app/utils.hh"
#include "log.hh"


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
            logger.log<ERROR>("Failed to get builder file {}: {}", p_file_path,
                              e.what().raw());
            std::terminate();
        }
    }


    auto
    get_toplevel(Gtk::Widget *p_obj) -> Gtk::Window *
    {
        return reinterpret_cast<Gtk::Window *>(p_obj->get_toplevel());
    }
}
