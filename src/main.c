#include <gtk/gtk.h>
#include <gtk4-layer-shell.h>
#include "window.h"
#include "config.h"
#include <stdio.h>

static void
on_activate (GtkApplication *app)
{
  g_print ("on_activate: Window displayed\n");
  
  HyprMenuWindow *window = hyprmenu_window_new (app);
  
  /* Show window */
  hyprmenu_window_show (window);
  
  g_print ("on_activate: Starting HyprMenu application\n");
}

static void
on_shutdown(GApplication *app, gpointer user_data)
{
  /* Free configuration */
  hyprmenu_config_free();
}

int
main (int argc, char *argv[])
{
  /* Initialize GTK */
  gtk_init();
  
  /* Initialize GTK Layer Shell */
  if (!gtk_layer_is_supported()) {
    g_critical("GTK Layer Shell not supported");
    return 1;
  }
  
  /* Initialize configuration */
  if (!hyprmenu_config_init()) {
    g_critical("Failed to initialize configuration");
    return 1;
  }
  
  /* Create application */
  GtkApplication *app = gtk_application_new ("org.hyprmenu.app", G_APPLICATION_FLAGS_NONE);
  g_signal_connect (app, "activate", G_CALLBACK (on_activate), NULL);
  g_signal_connect (app, "shutdown", G_CALLBACK (on_shutdown), NULL);
  
  /* Run application */
  int status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);
  
  return status;
} 