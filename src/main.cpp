#include<gtkmm.h>

import std;
import gui.status_bar;
import component.hypr;

int main(int argc, char *argv[]) {
  auto app = Gtk::Application::create("org.gtkmm.testing");

  app->signal_activate().connect([&](){
    auto r_builder = Gtk::Builder::create();
    try {
      r_builder->add_from_file("/home/seth/Development/wayland-status-bar/assets/interface.ui");
    } catch (const Glib::FileError& ex) {
      std::cerr << "FileError: " << ex.what() << std::endl;
      return;
    } catch (const Gtk::BuilderError& ex) {
      std::cerr << "BuilderError: " << ex.what() << std::endl;
      return;
    }

    auto p_status_bar = Gtk::Builder::get_widget_derived<StatusBar>(r_builder, "status_bar", StatusBar::BOTTOM);
    p_status_bar->set_visible(true);
    p_status_bar->override_style("/home/seth/Development/wayland-status-bar/assets/stylesheet.css");

    app->add_window(*p_status_bar);
  });

  return app->run(argc, argv);
}