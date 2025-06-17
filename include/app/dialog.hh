/**
 * aurgh Copyright (C) 2025 RQuarx
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
#pragma once
#ifndef __DIALOG__HH__
#define __DIALOG__HH__

#include <gtkmm/window.h>
#include <glibmm/main.h>
#include "package/type.hh"
#include "types.hh"

namespace Gtk { class Button; }


class Dialog : public Gtk::Window
{
public:
    Dialog(BaseObjectType  *cobject,
           const builder_t &b,
           const str       &p_title,
           const str       &p_message,
           bool             use_dark);

    auto wait_for_response() -> bool;

private:
    Gtk::Button
        *m_decline_button,
        *m_accept_button;

    Glib::RefPtr<Glib::MainLoop> m_loop;
    bool m_response = false;
};


#endif /* __DIALOG__HH__ _ */