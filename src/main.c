#include <gtk/gtk.h>
#include <gtk4-layer-shell.h>
#include <gdk/wayland/gdkwayland.h>
#include "window.h"
#include "config.h"

static void
setup_debug_logging(void)
{
  const char* debug_env = g_getenv("HYPRMENU_DEBUG");
  if (debug_env && *debug_env) {
    g_setenv("G_MESSAGES_DEBUG", "all", TRUE);
    g_message("Debug logging enabled");
  }
}

static void
on_activate(GtkApplication *app)
{
  g_message("Activating application");
  
  // Load config first
  if (!hyprmenu_config_load()) {
    g_critical("Failed to load configuration");
    return;
  }
  g_message("Configuration loaded successfully");
  
  // Create window
  HyprMenuWindow *window = hyprmenu_window_new(app);
  if (!window) {
    g_critical("Failed to create window");
    return;
  }
  g_message("Window created successfully");
  
  // Show window
  hyprmenu_window_show(window);
  g_message("Window shown");
}

static void
on_shutdown(GApplication *app, gpointer user_data)
{
  (void)app;  // Silence unused parameter warning
  (void)user_data;  // Silence unused parameter warning
  
  g_message("Shutting down application");
  hyprmenu_config_save();
  g_message("Configuration saved");
}

int
main(int argc, char *argv[])
{
  // Enable debug logging if requested
  setup_debug_logging();
  
  g_message("Starting HyprMenu");
  
  /* Force Wayland backend */
  g_setenv("GDK_BACKEND", "wayland", TRUE);
  g_message("Set Wayland backend");

  /* Initialize GTK */
  if (!gtk_init_check()) {
    g_critical("Failed to initialize GTK");
    return 1;
  }
  g_message("GTK initialized");

  /* Verify we're running under Wayland */
  GdkDisplay *display = gdk_display_get_default();
  if (!display) {
    g_critical("No display found");
    return 1;
  }
  
  if (!GDK_IS_WAYLAND_DISPLAY(display)) {
    g_critical("Not running under Wayland");
    return 1;
  }
  g_message("Confirmed running under Wayland");

  /* Check GTK Layer Shell */
  if (!gtk_layer_is_supported()) {
    g_critical("GTK Layer Shell is not supported by this compositor");
    return 1;
  }
  g_message("GTK Layer Shell support confirmed");
  
  // Create application
  GtkApplication *app = gtk_application_new("org.hyprmenu.app", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
  g_signal_connect(app, "shutdown", G_CALLBACK(on_shutdown), NULL);
  g_message("Application created");
  
  /* Initialize configuration */
  if (!hyprmenu_config_init()) {
    g_critical("Failed to initialize configuration");
    return 1;
  }
  g_message("Configuration initialized");
  
  // Run application
  g_message("Running application");
  int status = g_application_run(G_APPLICATION(app), argc, argv);
  
  // Cleanup
  g_object_unref(app);
  g_message("Application cleanup complete");
  
  return status;
} 