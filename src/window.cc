#include "logger.hh"
#include "window.hh"


AURWindow::AURWindow(std::array<Gtk::Box*, 2> tabs, Logger *logger) :
    m_tabs(tabs), m_logger(logger)
{
    set_title("AUR Gtk Helper");

    m_notebook = Gtk::make_managed<Gtk::Notebook>();
    for (Gtk::Box *t : m_tabs) {
        m_notebook->append_page(*t, "A");
    }

    add(*m_notebook);
    show_all();
}