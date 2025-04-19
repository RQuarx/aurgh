#pragma once
#ifndef WINDOW_HH__
#define WINDOW_HH__

#include <gtkmm-3.0/gtkmm/notebook.h>
#include <gtkmm-3.0/gtkmm/window.h>
#include <gtkmm-3.0/gtkmm/box.h>
#include <gtkmm-3.0/gtkmm/headerbar.h>
#include <gtkmm-3.0/gtkmm/image.h>
#include <gtkmm-3.0/gtkmm/cssprovider.h>
#include <gtkmm-3.0/gtkmm/stylecontext.h>
#include <gtkmm-3.0/gtkmm/icontheme.h>

class Logger;


class AURWindow : public Gtk::Window
{
public:
    explicit AURWindow(std::array<Gtk::Box*, 2> tabs, Logger *logger);

    auto render() -> bool;
private:
    std::array<Gtk::Box*, 2> m_tabs;
    Gtk::Notebook *m_notebook;
    Gtk::HeaderBar *m_header_bar;
    Glib::RefPtr<Gtk::CssProvider> m_css_provider;

    void load_css();
    void setup_header_bar();
    void setup_notebook_icons();

    Logger *m_logger;
};

#endif /* window.hh */