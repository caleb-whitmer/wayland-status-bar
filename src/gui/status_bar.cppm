module;

#include<gtkmm.h>
#include<gtk4-layer-shell.h> // basic C functionality to treat a GTK window as a
                             // status bar.

export module gui.status_bar;
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
    TOP = GTK_LAYER_SHELL_EDGE_TOP, 
    BOTTOM = GTK_LAYER_SHELL_EDGE_BOTTOM  
  };

  /**
   * @brief      Construct specifically for Gtk::Builder::get_widget_derived.
   *
   * @param      cobject  The underlying base C object.
   * @param[in]  builder  The gtk builder.
   */
  StatusBar(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
  : Gtk::Window(cobject),
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

      // Set the status bar margin to match the initial height of the window
      int width, height;
      this->get_default_size(width, height);
      gtk_layer_set_exclusive_zone(g_object_, height);
  }

  /**
   * @brief      Sets the location of the status bar on the screen.
   *
   * @param[in]  edge  The edge to which the status bar will be anchored.
   */
  void set_screen_location(ScreenLocation edge) {
    // Set the current edge anchor
    edge_anchor_ = edge; 
    
    // Because the status bar needs to be anchored exclusively to one edge, we
    // must first un-anchor it from all of the edges.
    for (auto e : { GTK_LAYER_SHELL_EDGE_LEFT, 
                    GTK_LAYER_SHELL_EDGE_RIGHT, 
                    GTK_LAYER_SHELL_EDGE_TOP, 
                    GTK_LAYER_SHELL_EDGE_BOTTOM }) {
      gtk_layer_set_anchor(g_object_, e, false);
    }

    // And then, anchor it exclusively to the desired edge.
    gtk_layer_set_anchor(
      g_object_, 
      static_cast<GtkLayerShellEdge>(edge_anchor_), 
      true);
  }

  /**
   * @brief      Gets the location of the status bar on the screen (top or
   *             bottom).
   *
   * @return     The screen location.
   */
  inline const ScreenLocation get_screen_location(void) const {
    return edge_anchor_;
  }

  /**
   * @brief      Overrides the default style given to the application.
   *
   * @param[in]  css   The CSS used to override the default style.
   */
  inline void override_style(const std::string& css) {
    r_css_provider_->load_from_data(css);
  }
 protected:
  Glib::RefPtr<Gtk::CssProvider> r_css_provider_;
 private:
  ScreenLocation edge_anchor_;
  GtkWindow* g_object_;
};