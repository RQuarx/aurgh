#include <glibmm.h>
#include <gtkmm/builder.h>

#include "app/utils.hh"
#include "logger.hh"


namespace app
{
    auto
    get_builder(const std::string &file_path) -> Glib::RefPtr<Gtk::Builder>
    {
        try
        {
            return Gtk::Builder::create_from_resource(file_path);
        }
        catch (const Glib::Error &e)
        {
            logger[Level::ERROR, "app::utils"](
                "Failed to get builder file {}: {}", file_path,
                e.what().raw());
            std::terminate();
        }
    }
}
