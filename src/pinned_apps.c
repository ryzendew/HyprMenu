#include "pinned_apps.h"
#include "app_entry.h"
#include "config.h"
#include <errno.h>

#define PINNED_APPS_FILE ".config/hyprmenu/pinned.txt"

struct _HyprMenuPinnedApps
{
  GtkBox parent_instance;
  
  GtkWidget *label;
  GtkWidget *apps_box;
  GList *pinned_ids;
};

G_DEFINE_TYPE (HyprMenuPinnedApps, hyprmenu_pinned_apps, GTK_TYPE_BOX)

static void
load_pinned_apps (HyprMenuPinnedApps *self)
{
  g_print("Loading pinned apps\n");
  
  g_list_free_full (self->pinned_ids, g_free);
  self->pinned_ids = NULL;
  
  /* Load pinned apps from file */
  char *path = g_build_filename (g_get_home_dir (), PINNED_APPS_FILE, NULL);
  g_print("Checking for pinned apps file: %s\n", path);
  
  if (g_file_test (path, G_FILE_TEST_EXISTS)) {
    g_print("Found pinned apps file, loading...\n");
    char *contents = NULL;
    gsize length = 0;
    GError *error = NULL;
    
    if (g_file_get_contents (path, &contents, &length, &error)) {
      /* Split by newline and add to list */
      char **lines = g_strsplit (contents, "\n", -1);
      int count = 0;
      
      for (int i = 0; lines[i] != NULL && lines[i][0] != '\0'; i++) {
        g_print("Found pinned app: %s\n", lines[i]);
        self->pinned_ids = g_list_append (self->pinned_ids, g_strdup (lines[i]));
        count++;
      }
      
      g_print("Loaded %d pinned apps\n", count);
      g_strfreev (lines);
      g_free (contents);
    } else {
      g_warning("Failed to read pinned apps file: %s", error->message);
      g_error_free(error);
    }
  } else {
    g_print("Pinned apps file not found, no apps loaded\n");
  }
  
  g_free (path);
}

static void
save_pinned_apps (HyprMenuPinnedApps *self)
{
  g_print("Saving pinned apps to file\n");
  
  /* Create directory if it doesn't exist - force all directories in path */
  char *config_dir = g_build_filename (g_get_home_dir (), ".config", NULL);
  g_print("Creating .config directory if needed: %s\n", config_dir);
  
  // Create .config directory first
  if (g_mkdir_with_parents (config_dir, 0755) == -1) {
    g_warning("Failed to create .config directory: %s (error: %s)", config_dir, g_strerror(errno));
  }
  g_free (config_dir);
  
  // Now create hyprmenu subdirectory
  char *dir = g_build_filename (g_get_home_dir (), ".config/hyprmenu", NULL);
  g_print("Creating hyprmenu directory if needed: %s\n", dir);
  
  if (g_mkdir_with_parents (dir, 0755) == -1) {
    g_warning("Failed to create hyprmenu directory: %s (error: %s)", dir, g_strerror(errno));
  }
  g_free (dir);
  
  /* Create file content */
  GString *content = g_string_new ("");
  int count = 0;
  
  for (GList *l = self->pinned_ids; l != NULL; l = l->next) {
    const char *app_id = (const char *) l->data;
    g_print("Adding app to save content: %s\n", app_id);
    g_string_append (content, app_id);
    g_string_append (content, "\n");
    count++;
  }
  
  /* Save to file */
  char *path = g_build_filename (g_get_home_dir (), PINNED_APPS_FILE, NULL);
  g_print("Saving %d pinned apps to file: %s\n", count, path);
  
  // Try multiple methods to ensure file is created
  gboolean success = FALSE;
  
  // Method 1: g_file_set_contents
  GError *error = NULL;
  if (g_file_set_contents (path, content->str, -1, &error)) {
    g_print("Pinned apps saved successfully using g_file_set_contents\n");
    success = TRUE;
  } else {
    g_warning("Failed to save pinned apps file using g_file_set_contents: %s (error: %s)", 
              path, error->message);
    g_error_free(error);
    
    // Method 2: Direct file writing with fopen/fputs
    g_print("Attempting direct file write as fallback...\n");
    FILE *file = fopen(path, "w");
    if (file) {
      if (fputs(content->str, file) >= 0) {
        g_print("Pinned apps saved successfully using direct file write\n");
        success = TRUE;
      } else {
        g_warning("Failed to write to pinned apps file: %s (error: %s)", 
                  path, g_strerror(errno));
      }
      fclose(file);
    } else {
      g_warning("Failed to open pinned apps file for writing: %s (error: %s)", 
                path, g_strerror(errno));
      
      // Method 3: System command as last resort
      g_print("Attempting system command as last resort...\n");
      char *cmd = g_strdup_printf("mkdir -p \"%s/.config/hyprmenu\" && "
                                 "echo \"%s\" > \"%s\"",
                                 g_get_home_dir(), content->str, path);
      int ret = system(cmd);
      if (ret == 0) {
        g_print("Pinned apps saved successfully using system command\n");
        success = TRUE;
      } else {
        g_warning("Failed to save pinned apps file using system command (ret=%d)", ret);
      }
      g_free(cmd);
    }
  }
  
  if (success) {
    g_print("Pinned apps saved successfully\n");
  } else {
    g_warning("All attempts to save pinned apps failed\n");
  }
  
  g_string_free (content, TRUE);
  g_free (path);
}

static void
hyprmenu_pinned_apps_finalize (GObject *object)
{
  HyprMenuPinnedApps *self = HYPRMENU_PINNED_APPS (object);
  
  g_list_free_full (self->pinned_ids, g_free);
  
  G_OBJECT_CLASS (hyprmenu_pinned_apps_parent_class)->finalize (object);
}

// Add this function for handling direct clicks on the flowbox
static void
on_pinned_apps_clicked(GtkGestureClick *gesture,
                      gint n_press,
                      double x,
                      double y,
                      gpointer user_data)
{
  (void)n_press;  // Silence unused parameter warning
  
  g_print("DEBUG: on_pinned_apps_clicked() called at x=%.1f, y=%.1f\n", x, y);
  
  HyprMenuPinnedApps *self = HYPRMENU_PINNED_APPS(user_data);
  if (!self || !self->apps_box) {
    g_warning("LAUNCH ERROR: Invalid self or apps_box in on_pinned_apps_clicked");
    return;
  }
  
  // Get the child at the click position
  GtkFlowBoxChild *child = gtk_flow_box_get_child_at_pos(GTK_FLOW_BOX(self->apps_box), x, y);
  
  if (child) {
    g_print("DEBUG: Found pinned apps flow box child at position\n");
    
    // Manually activate the child
    GtkWidget *app_widget = gtk_flow_box_child_get_child(child);
    if (app_widget && HYPRMENU_IS_APP_ENTRY(app_widget)) {
      g_print("DEBUG: Manually launching pinned app\n");
      hyprmenu_app_entry_launch(HYPRMENU_APP_ENTRY(app_widget));
    }
  } else {
    g_print("DEBUG: No pinned apps flow box child found at position\n");
  }
}

// Add this function to handle flowbox child activation
static void
on_pinned_app_activated(GtkFlowBox *flowbox,
                       GtkFlowBoxChild *child,
                       gpointer user_data)
{
  (void)flowbox;  // Silence unused parameter warning
  (void)user_data;  // Silence unused parameter warning
  
  g_print("DEBUG: on_pinned_app_activated() called\n");
  
  if (!child) {
    g_warning("LAUNCH ERROR: child is NULL in on_pinned_app_activated");
    return;
  }
  
  GtkWidget *app_widget = gtk_flow_box_child_get_child(child);
  
  if (!app_widget) {
    g_warning("LAUNCH ERROR: app_widget is NULL in on_pinned_app_activated");
    return;
  }
  
  if (HYPRMENU_IS_APP_ENTRY(app_widget)) {
    g_print("DEBUG: Launching pinned app\n");
    hyprmenu_app_entry_launch(HYPRMENU_APP_ENTRY(app_widget));
  } else {
    g_warning("LAUNCH ERROR: app_widget is not an HyprMenuAppEntry");
  }
}

static void
hyprmenu_pinned_apps_init (HyprMenuPinnedApps *self)
{
  /* Create UI */
  GtkWidget *box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 8);
  gtk_widget_set_parent (box, GTK_WIDGET (self));
  
  /* Add title */
  self->label = gtk_label_new (NULL);
  char *markup = g_markup_printf_escaped("<span weight='bold' size='larger'>%s</span>", 
                                       config->pinned_apps_title);
  gtk_label_set_markup (GTK_LABEL (self->label), markup);
  g_free (markup);
  
  gtk_widget_add_css_class (self->label, "hyprmenu-category-title");
  gtk_label_set_xalign (GTK_LABEL (self->label), 0);
  gtk_widget_set_margin_start(self->label, config->category_padding);
  gtk_widget_set_margin_top (self->label, 8);
  gtk_widget_set_margin_bottom (self->label, 8);
  
  gtk_box_append (GTK_BOX (box), self->label);
  
  /* Add flow box for apps */
  self->apps_box = gtk_flow_box_new ();
  gtk_flow_box_set_selection_mode (GTK_FLOW_BOX (self->apps_box), GTK_SELECTION_NONE);
  gtk_flow_box_set_homogeneous (GTK_FLOW_BOX (self->apps_box), FALSE);
  gtk_flow_box_set_activate_on_single_click (GTK_FLOW_BOX (self->apps_box), TRUE);
  
  g_print("DEBUG: Setting up pinned apps flowbox with child-activated signal\n");
  
  // Connect child-activated signal to launch apps
  g_signal_connect(self->apps_box, "child-activated", G_CALLBACK(on_pinned_app_activated), NULL);
  
  // Add direct click handler for additional reliability
  GtkGesture *click_gesture = gtk_gesture_click_new();
  gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(click_gesture), GDK_BUTTON_PRIMARY);
  g_signal_connect(click_gesture, "pressed", G_CALLBACK(on_pinned_apps_clicked), self);
  gtk_widget_add_controller(self->apps_box, GTK_EVENT_CONTROLLER(click_gesture));
  
  int columns = config->grid_hexpand ? config->grid_columns : 5;
  gtk_flow_box_set_max_children_per_line (GTK_FLOW_BOX (self->apps_box), columns);
  gtk_flow_box_set_min_children_per_line (GTK_FLOW_BOX (self->apps_box), columns);
  
  gtk_widget_add_css_class (self->apps_box, "hyprmenu-pinned-apps");
  gtk_widget_set_margin_start (self->apps_box, config->category_padding);
  gtk_widget_set_margin_end (self->apps_box, config->category_padding);
  
  /* Set hexpand and halign based on config */
  gtk_widget_set_hexpand (self->apps_box, config->grid_hexpand);
  if (config->grid_hexpand) {
    gtk_widget_set_halign (self->apps_box, GTK_ALIGN_CENTER);
  } else {
    gtk_widget_set_halign (self->apps_box, GTK_ALIGN_START);
  }
  
  gtk_box_append (GTK_BOX (box), self->apps_box);
  
  /* Initialize data */
  self->pinned_ids = NULL;
  load_pinned_apps (self);
}

static void
hyprmenu_pinned_apps_class_init (HyprMenuPinnedAppsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  
  object_class->finalize = hyprmenu_pinned_apps_finalize;
}

HyprMenuPinnedApps *
hyprmenu_pinned_apps_new (void)
{
  return g_object_new (HYPRMENU_TYPE_PINNED_APPS, NULL);
}

void
hyprmenu_pinned_apps_refresh (HyprMenuPinnedApps *self)
{
  g_print("Refreshing pinned apps...\n");

  /* Remove existing widgets */
  GtkWidget *child;
  while ((child = gtk_widget_get_first_child (self->apps_box)) != NULL) {
    gtk_flow_box_remove (GTK_FLOW_BOX (self->apps_box), child);
  }
  
  /* Add apps from pinned list */
  int count = 0;
  for (GList *l = self->pinned_ids; l != NULL; l = l->next) {
    const char *app_id = (const char *) l->data;
    g_print("Processing pinned app: %s\n", app_id);
    
    GDesktopAppInfo *app_info = g_desktop_app_info_new (app_id);
    
    if (app_info) {
      HyprMenuAppEntry *entry = hyprmenu_app_entry_new (app_info);
      if (entry) {
        // Set grid layout based on config
        hyprmenu_app_entry_set_grid_layout (entry, config->grid_hexpand);
        // Set icon size proportional to grid item size
        if (config->grid_hexpand) {
          hyprmenu_app_entry_set_icon_size (entry, config->grid_item_size * 0.6);
        }
        gtk_flow_box_append (GTK_FLOW_BOX (self->apps_box), GTK_WIDGET (entry));
        count++;
      }
      g_object_unref (app_info);
    } else {
      g_warning("Failed to load app info for %s", app_id);
    }
  }
  
  g_print("Found %d pinned apps\n", count);
  
  /* Always show container initially, even if empty */
  gtk_widget_set_visible (GTK_WIDGET (self), TRUE);
  
  /* Hide the pinned apps section only if there are no pinned apps */
  if (g_list_length (self->pinned_ids) == 0) {
    g_print("No pinned apps found, hiding section\n");
    gtk_widget_set_visible (GTK_WIDGET (self), FALSE);
  } else {
    g_print("Showing pinned apps section with %d apps\n", count);
    // Make sure children are visible
    gtk_widget_set_visible (self->label, TRUE);
    gtk_widget_set_visible (self->apps_box, TRUE);
  }
}

void
hyprmenu_pinned_apps_add_app (HyprMenuPinnedApps *self, const char *app_id)
{
  if (!app_id || app_id[0] == '\0') {
    g_warning("Attempted to pin app with null or empty app_id");
    return;
  }

  g_print("Adding app to pinned: %s\n", app_id);
  
  /* Check if already in list */
  if (hyprmenu_pinned_apps_has_app(self, app_id)) {
    g_print("App already pinned: %s\n", app_id);
    return; // Already pinned
  }
  
  /* Add to list */
  self->pinned_ids = g_list_append (self->pinned_ids, g_strdup (app_id));
  g_print("Added app to pinned list: %s\n", app_id);
  
  /* Save to file */
  save_pinned_apps (self);
  
  /* Refresh display */
  hyprmenu_pinned_apps_refresh (self);
}

gboolean
hyprmenu_pinned_apps_remove_app (HyprMenuPinnedApps *self, const char *app_id)
{
  /* Find in list */
  GList *existing = g_list_find_custom (self->pinned_ids, app_id, (GCompareFunc) g_strcmp0);
  if (!existing) {
    return FALSE; // Not found
  }
  
  /* Remove from list */
  g_free (existing->data);
  self->pinned_ids = g_list_delete_link (self->pinned_ids, existing);
  
  /* Save to file */
  save_pinned_apps (self);
  
  /* Refresh display */
  hyprmenu_pinned_apps_refresh (self);
  
  return TRUE;
}

gboolean
hyprmenu_pinned_apps_has_app (HyprMenuPinnedApps *self, const char *app_id)
{
  if (!app_id) {
    g_print("HAS_APP: app_id is NULL\n");
    return FALSE;
  }
  
  g_print("HAS_APP: Checking if app is pinned: '%s'\n", app_id);
  
  // Manual loop for detailed comparison
  for (GList *l = self->pinned_ids; l != NULL; l = l->next) {
    const char *pinned_id = (const char *)l->data;
    
    if (!pinned_id) continue; // Skip NULL entries
    
    g_print("HAS_APP: Comparing with pinned ID: '%s'\n", pinned_id);
    
    // Simple exact string comparison
    if (g_strcmp0(app_id, pinned_id) == 0) {
      g_print("HAS_APP: Exact match found\n");
      return TRUE;
    }
    
    // Try a case-insensitive comparison as a fallback
    if (g_ascii_strcasecmp(app_id, pinned_id) == 0) {
      g_print("HAS_APP: Case-insensitive match found\n");
      return TRUE;
    }
    
    // Check if one is a prefix of the other (handle possible truncation)
    if (g_str_has_prefix(app_id, pinned_id) || g_str_has_prefix(pinned_id, app_id)) {
      g_print("HAS_APP: Prefix match found\n");
      return TRUE;
    }
  }
  
  g_print("HAS_APP: No match found\n");
  return FALSE;
} 