#include <gtkmm/application.h>

#include "window.hh"


auto
main(int argc, char **argv) -> int
{
    return Gtk::Application::create("org.kei.aurgh")
        ->make_window_and_run<aurgh::window>(argc, argv);
}
