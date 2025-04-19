#include "logger.hh"
#include "window.hh"


AURWindow::AURWindow(std::array<Gtk::Box*, 2> tabs, Logger *logger) :
    m_tabs(tabs),
    m_notebook(Gtk::make_managed<Gtk::Notebook>()),
    m_logger(logger)
{
    set_title("AUR Gtk Helper");

    m_notebook->append_page(*tabs.at(0), "Packages");
    m_notebook->append_page(*tabs.at(1), "Management");

    add(*m_notebook);
    show_all();
}