#pragma once
#include <gtkmm/builder.h>

#include "result.hh"


namespace aurgh::widget
{
    class base
    {
    public:
        auto
        build(this auto &self, const Glib::RefPtr<Gtk::Builder> &builder) noexcept -> result<void>
        { self.build(builder); }
    };
}
