#include <print>

#include <gtkmm.h>

#include "window.hh"

using aurgh::window;


window::window()
    : m_client {
          client::create(http::client::create().value(), "/home/kei/project/aurgh/clone/").value()
      }
{
    auto builder = Gtk::Builder::create_from_resource("/org/kei/aurgh/window.ui");

    this->set_title("aurgh");
    this->set_child(*builder->get_widget<Gtk::Box>("main_container"));

    if (auto res = m_searchbar.build(builder); !res) { /* TODO: Handle error */ }

    if (auto res = m_client->clone("https://github.com/RQuarx/aurgh.git"); res.has_value())
    {
        auto &ref = res.value().get();
        ref.signal_on_clone_complete().connect(
            [](result<std::filesystem::path> dst)
            {
                if (dst.has_value())
                    std::println("clone completed on {}", dst->c_str());
                else
                    std::println("error: {}", dst.error());
            });
        ref.signal_on_clone_progress().connect([](double prog) { std::println("{:.2}", prog); });
    }
    else
        std::println("error: {}", res.error());
}
