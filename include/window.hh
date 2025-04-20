/**
 * @file window.hh
 *
 * This file is part of AURGH
 *
 * AURGH is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * AURGH is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar. If not, see <https://www.gnu.org/licenses/>.
 */

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