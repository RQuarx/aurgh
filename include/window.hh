#pragma once
#ifndef WINDOW_HH__
#define WINDOW_HH__

#include <gtkmm-3.0/gtkmm/notebook.h>
#include <gtkmm-3.0/gtkmm/window.h>
#include <gtkmm-3.0/gtkmm/box.h>

class Logger;


class AURWindow : public Gtk::Window
{
public:
    explicit AURWindow(std::array<Gtk::Box*, 2> tabs, Logger *logger);

    auto render() -> bool;
private:
    std::array<Gtk::Box*, 2> m_tabs;
    Gtk::Notebook *m_notebook;

    Logger *m_logger;
};

#endif /* window.hh */