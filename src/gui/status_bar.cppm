module;

#include<gtkmm.h>
#include<gtk4-layer-shell.h> // basic C functionality to treat a GTK window as a
                             // status bar.

export module gui.status_bar;
import component.hypr;
import std;

/**
 * @brief      The base window for the status bar. Appears at either the top or
 *             bottom of the screen and contains all status bar widgets.
 */
export class StatusBar : public Gtk::Window {
 public:
  /**
   * @brief      The status bar can be placed either on the top or the bottom of
   *             the screen.
   */
  enum ScreenLocation { 
    TOP     = GTK_LAYER_SHELL_EDGE_TOP, 
    BOTTOM  = GTK_LAYER_SHELL_EDGE_BOTTOM  
  };

  /**
   * @brief      Construct specifically for Gtk::Builder::get_widget_derived.
   *
   * @param      cobject  The underlying base C object.
   * @param[in]  builder  The gtk builder.
   */
  StatusBar(  BaseObjectType* cobject, 
              const Glib::RefPtr<Gtk::Builder>& builder, 
              ScreenLocation edge)
  : Gtk::Window(cobject),
    r_builder_(builder),
    r_css_provider_(Gtk::CssProvider::create()),
    g_object_{this->gobj()} {


      // Initialize the style provider to override the base system styling.
      Gtk::StyleProvider::add_provider_for_display(
        this->get_display(),
        r_css_provider_,
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);


      // Place the status bar on the top layer of the screen
      gtk_layer_init_for_window(g_object_);
      gtk_layer_set_layer(g_object_, GTK_LAYER_SHELL_LAYER_TOP);
      
      // Set the location on the screen to which the status bar will be
      // anchored
      gtk_layer_set_anchor( g_object_, 
                            static_cast<GtkLayerShellEdge>(edge), 
                            true);

      // Get the outer margins from hyprland
      std::string hypr_gaps_out = hypr::get_option("general:gaps_out")["css"];
      struct { unsigned t, r, b, l; } margin;
      std::sscanf(hypr_gaps_out.c_str(), 
                  "%u %u %u %u", 
                  &margin.t, 
                  &margin.r, 
                  &margin.b, 
                  &margin.l);

      bool is_top = (edge == ScreenLocation::TOP);

      // Get the default size of the status bar
      int width, height;
      this->get_default_size(width, height);

      // Increase the height of the status bar by the required margin
      int backing_height = height + (is_top ? margin.t : margin.b);
      gtk_layer_set_exclusive_zone(g_object_, backing_height);

      // Get the base frame of the status bar
      auto p_base_frame = r_builder_->get_widget<Gtk::ScrolledWindow>("base_frame");
      if (!p_base_frame)
        throw std::runtime_error("Incomplete widget layout!");

      // Align the base_frame to the top of the bar if anchored to the top,
      // and to the bottom of the bar if anchored to the bottom of the screen
      p_base_frame->set_valign(is_top ? Gtk::Align::START : Gtk::Align::END);

      // Set the height of the base frame to match the desired height of the
      // status bar
      p_base_frame->set_size_request(-1, height);

      // Match the corrosponding margin with that of the hyprland config
      if (is_top)
        p_base_frame->set_margin_top(margin.t);
      else
        p_base_frame->set_margin_bottom(margin.b);
  }

  /**
   * @brief      Override the default styling of the application.
   *
   * @param[in]  path  The path to the requested stylesheet.
   */
  void override_style(const std::string& path) {
    try {
      r_css_provider_->load_from_path(path);
    } catch(const Glib::FileError& ex) {
      throw ex;
    } 
  }
 protected:
  Glib::RefPtr<Gtk::Builder> r_builder_;
  Glib::RefPtr<Gtk::CssProvider> r_css_provider_;
 private:
  ScreenLocation edge_anchor_;
  GtkWindow* g_object_;
};