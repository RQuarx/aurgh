/**
 * @file window.cc
 *
 * This file is part of aurgh
 *
 * aurgh is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * aurgh is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with aurgh. If not, see <https://www.gnu.org/licenses/>.
 */

#include "logger.hh"
#include "window.hh"


AURWindow::AURWindow(std::array<Gtk::Box*, 2> tabs, Logger *logger) :
    m_tabs(tabs),
    // m_notebook(Gtk::make_managed<Gtk::Notebook>()),
    m_logger(logger)
{
    set_title("AUR Gtk Helper");

    // m_notebook->append_page(*tabs.at(0), "Packages");
    // m_notebook->append_page(*tabs.at(1), "Management");

    // add(*m_notebook);
    add(*tabs.at(0));
    show_all();
}