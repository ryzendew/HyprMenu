#include "recent_apps.h"
#include "app_entry.h"
#include "config.h"

#define RECENT_APPS_FILE ".config/hyprmenu/recent.txt"

struct _HyprMenuRecentApps
{
  GtkBox parent_instance;
  
  GtkWidget *label;
  GtkWidget *apps_box;
  GList *recent_ids;
};

G_DEFINE_TYPE (HyprMenuRecentApps, hyprmenu_recent_apps, GTK_TYPE_BOX)

static void
load_recent_apps (HyprMenuRecentApps *self)
{
  g_list_free_full (self->recent_ids, g_free);
  self->recent_ids = NULL;
  
  /* Load recent apps from file */
  char *path = g_build_filename (g_get_home_dir (), RECENT_APPS_FILE, NULL);
  
  if (g_file_test (path, G_FILE_TEST_EXISTS)) {
    char *contents = NULL;
    gsize length = 0;
    
    if (g_file_get_contents (path, &contents, &length, NULL)) {
      /* Split by newline and add to list */
      char **lines = g_strsplit (contents, "\n", -1);
      
      for (int i = 0; lines[i] != NULL && lines[i][0] != '\0'; i++) {
        /* Limit number of recent apps */
        if (g_list_length (self->recent_ids) >= config->max_recent_apps)
          break;
          
        self->recent_ids = g_list_append (self->recent_ids, g_strdup (lines[i]));
      }
      
      g_strfreev (lines);
      g_free (contents);
    }
  }
  
  g_free (path);
}

static void
save_recent_apps (HyprMenuRecentApps *self)
{
  /* Create directory if it doesn't exist */
  char *dir = g_build_filename (g_get_home_dir (), ".config/hyprmenu", NULL);
  g_mkdir_with_parents (dir, 0755);
  g_free (dir);
  
  /* Create file content */
  GString *content = g_string_new ("");
  
  for (GList *l = self->recent_ids; l != NULL; l = l->next) {
    g_string_append (content, (char *) l->data);
    g_string_append (content, "\n");
  }
  
  /* Save to file */
  char *path = g_build_filename (g_get_home_dir (), RECENT_APPS_FILE, NULL);
  g_file_set_contents (path, content->str, -1, NULL);
  
  g_string_free (content, TRUE);
  g_free (path);
}

static void
hyprmenu_recent_apps_finalize (GObject *object)
{
  HyprMenuRecentApps *self = HYPRMENU_RECENT_APPS (object);
  
  g_list_free_full (self->recent_ids, g_free);
  
  G_OBJECT_CLASS (hyprmenu_recent_apps_parent_class)->finalize (object);
}

static void
on_recent_apps_clicked(GtkGestureClick *gesture,
                      gint n_press,
                      double x,
                      double y,
                      gpointer user_data)
{
  (void)n_press;  // Silence unused parameter warning
  
  g_print("DEBUG: on_recent_apps_clicked() called at x=%.1f, y=%.1f\n", x, y);
  
  HyprMenuRecentApps *self = HYPRMENU_RECENT_APPS(user_data);
  if (!self || !self->apps_box) {
    g_warning("LAUNCH ERROR: Invalid self or apps_box in on_recent_apps_clicked");
    return;
  }
  
  // Get the child at the click position
  GtkFlowBoxChild *child = gtk_flow_box_get_child_at_pos(GTK_FLOW_BOX(self->apps_box), x, y);
  
  if (child) {
    g_print("DEBUG: Found recent apps flow box child at position\n");
    
    // Manually activate the child
    GtkWidget *app_widget = gtk_flow_box_child_get_child(child);
    if (app_widget && HYPRMENU_IS_APP_ENTRY(app_widget)) {
      g_print("DEBUG: Manually launching recent app\n");
      hyprmenu_app_entry_launch(HYPRMENU_APP_ENTRY(app_widget));
    }
  } else {
    g_print("DEBUG: No recent apps flow box child found at position\n");
  }
}

static void
on_recent_app_activated(GtkFlowBox *flowbox,
                       GtkFlowBoxChild *child,
                       gpointer user_data)
{
  (void)flowbox;  // Silence unused parameter warning
  (void)user_data;  // Silence unused parameter warning
  
  g_print("DEBUG: on_recent_app_activated() called\n");
  
  if (!child) {
    g_warning("LAUNCH ERROR: child is NULL in on_recent_app_activated");
    return;
  }
  
  GtkWidget *app_widget = gtk_flow_box_child_get_child(child);
  
  if (!app_widget) {
    g_warning("LAUNCH ERROR: app_widget is NULL in on_recent_app_activated");
    return;
  }
  
  if (HYPRMENU_IS_APP_ENTRY(app_widget)) {
    g_print("DEBUG: Launching recent app\n");
    hyprmenu_app_entry_launch(HYPRMENU_APP_ENTRY(app_widget));
  } else {
    g_warning("LAUNCH ERROR: app_widget is not an HyprMenuAppEntry");
  }
}

static void
hyprmenu_recent_apps_init (HyprMenuRecentApps *self)
{
  /* Create UI */
  GtkWidget *box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 8);
  gtk_widget_set_parent (box, GTK_WIDGET (self));
  
  /* Add title */
  self->label = gtk_label_new (NULL);
  char *markup = g_markup_printf_escaped("<span weight='bold' size='larger'>%s</span>", 
                                       config->recent_apps_title);
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
  
  g_print("DEBUG: Setting up recent apps flowbox with child-activated signal\n");
  
  // Connect child-activated signal to launch apps
  g_signal_connect(self->apps_box, "child-activated", G_CALLBACK(on_recent_app_activated), NULL);
  
  // Add direct click handler for additional reliability
  GtkGesture *click_gesture = gtk_gesture_click_new();
  gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(click_gesture), GDK_BUTTON_PRIMARY);
  g_signal_connect(click_gesture, "pressed", G_CALLBACK(on_recent_apps_clicked), self);
  gtk_widget_add_controller(self->apps_box, GTK_EVENT_CONTROLLER(click_gesture));
  
  int columns = config->grid_hexpand ? config->grid_columns : 5;
  gtk_flow_box_set_max_children_per_line (GTK_FLOW_BOX (self->apps_box), columns);
  gtk_flow_box_set_min_children_per_line (GTK_FLOW_BOX (self->apps_box), columns);
  
  gtk_widget_add_css_class (self->apps_box, "hyprmenu-recent-apps");
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
  self->recent_ids = NULL;
  load_recent_apps (self);
}

static void
hyprmenu_recent_apps_class_init (HyprMenuRecentAppsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  
  object_class->finalize = hyprmenu_recent_apps_finalize;
}

HyprMenuRecentApps *
hyprmenu_recent_apps_new (void)
{
  return g_object_new (HYPRMENU_TYPE_RECENT_APPS, NULL);
}

void
hyprmenu_recent_apps_refresh (HyprMenuRecentApps *self)
{
  /* Remove existing widgets */
  GtkWidget *child;
  while ((child = gtk_widget_get_first_child (self->apps_box)) != NULL) {
    gtk_flow_box_remove (GTK_FLOW_BOX (self->apps_box), child);
  }
  
  /* Add apps from recent list */
  for (GList *l = self->recent_ids; l != NULL; l = l->next) {
    const char *app_id = (const char *) l->data;
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
      }
      g_object_unref (app_info);
    }
  }
  
  /* Hide if no recent apps */
  if (g_list_length (self->recent_ids) == 0) {
    gtk_widget_set_visible (GTK_WIDGET (self), FALSE);
  } else {
    gtk_widget_set_visible (GTK_WIDGET (self), TRUE);
  }
}

void
hyprmenu_recent_apps_add_app (HyprMenuRecentApps *self, const char *app_id)
{
  /* Remove if already in list */
  GList *existing = g_list_find_custom (self->recent_ids, app_id, (GCompareFunc) g_strcmp0);
  if (existing) {
    g_free (existing->data);
    self->recent_ids = g_list_delete_link (self->recent_ids, existing);
  }
  
  /* Add to beginning of list */
  self->recent_ids = g_list_prepend (self->recent_ids, g_strdup (app_id));
  
  /* Trim list if needed */
  while (g_list_length (self->recent_ids) > config->max_recent_apps) {
    GList *last = g_list_last (self->recent_ids);
    g_free (last->data);
    self->recent_ids = g_list_delete_link (self->recent_ids, last);
  }
  
  /* Save to file */
  save_recent_apps (self);
  
  /* Refresh display */
  hyprmenu_recent_apps_refresh (self);
} 