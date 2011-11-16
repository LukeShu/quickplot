/*
  Quickplot - an interactive 2D plotter

  Copyright (C) 1998-2011  Lance Arsenault


  This file is part of Quickplot.

  Quickplot is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published
  by the Free Software Foundation, either version 3 of the License,
  or (at your option) any later version.

  Quickplot is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Quickplot.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>


#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "quickplot.h"

#include "config.h"
#include "debug.h"
#include "list.h"
#include "qp.h"
#include "callbacks.h"

#include "imgGrabCursor.xpm"
#include "imgHoldCursor.xpm"
#include "quickplot_icon.xpm"
#include "imgNewWindow.xpm"
#include "imgDeleteWindow.xpm"
#include "imgCopyWindow.xpm"
#include "imgSaveImage.xpm"

static int main_window_create_count = 0;


static inline
GtkWidget *create_check_menu_item(GtkWidget *menu, const char *label,
    guint key, gboolean is_checked,
    void (*callback)(GtkWidget*, void*),
    gpointer data)
{
  GtkWidget *mi;
  mi = gtk_check_menu_item_new_with_mnemonic(label);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), mi);

  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mi), is_checked);

  if(key)
    gtk_widget_add_accelerator(mi, "activate",
      gtk_menu_get_accel_group(GTK_MENU(menu)),
      key, (GdkModifierType) 0x0, GTK_ACCEL_VISIBLE);

  g_signal_connect(G_OBJECT(mi), "toggled", G_CALLBACK(callback), data);

  gtk_widget_show(mi);
  return mi;
}

    
static inline
void create_menu_item_seperator(GtkWidget *menu)
{
  GtkWidget *mi;
  mi = gtk_separator_menu_item_new();
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), mi);
  gtk_widget_show(mi);
}

static inline
GtkWidget *create_menu(GtkWidget *menubar,
    GtkAccelGroup *accelGroup, const char *label)
{
  GtkWidget *menuitem, *menu;
  menuitem = gtk_menu_item_new_with_label(label);
  menu = gtk_menu_new();
  gtk_menu_set_accel_group(GTK_MENU(menu), accelGroup);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem), menu);
  gtk_menu_shell_append(GTK_MENU_SHELL(menubar), menuitem);
  gtk_widget_show(menuitem);
  /* The menu is shown when the user activates it. */
  return menu;
}

static
void dummy_callback(GtkWidget *w, void *ptr)
{
  NOTICE("%s(w=%p, ptr=%p)\n",__func__, w, ptr);
}

static inline
GtkWidget *create_menu_item(GtkWidget *menu,
	 const char *label,
	 const char *pixmap[], const gchar *STOCK,
	 guint key, gboolean with_mnemonic,
	 void (*callback)(GtkWidget*, void*),
	 gpointer data, gboolean is_sensitive)
{
  GtkWidget *mi;

  if(with_mnemonic)
    mi = gtk_image_menu_item_new_with_mnemonic(label);
  else
    mi =  gtk_image_menu_item_new_with_label(label);

  if(pixmap)
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(mi),
      gtk_image_new_from_pixbuf(
	gdk_pixbuf_new_from_xpm_data(pixmap)));
  else if(STOCK)
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(mi),
      gtk_image_new_from_stock(STOCK, GTK_ICON_SIZE_MENU));

  gtk_menu_shell_append(GTK_MENU_SHELL(menu), mi);

  if(callback)
    g_signal_connect(G_OBJECT(mi), "activate", G_CALLBACK(callback), data);

  if(key)
    gtk_widget_add_accelerator(mi, "activate",
      gtk_menu_get_accel_group(GTK_MENU(menu)),
      key, (GdkModifierType) 0x0, GTK_ACCEL_VISIBLE);

  gtk_widget_set_sensitive(mi, is_sensitive);
  gtk_widget_show(mi);
  return mi;
}

static inline
void add_source_buffer_remove_menu(struct qp_qp *qp, struct qp_source *s)
{
  GtkWidget *mi;
  const size_t MLEN = 64;
  char str[MLEN];
  char *name;
  size_t len;
  len = strlen(s->name);
  if(len > MLEN-1)
  {
    snprintf(str, MLEN, "... %s", &s->name[len-59]);
    name = str;
  }
  else
    name = s->name;

  mi = create_menu_item(qp->file_menu, name, NULL, GTK_STOCK_DELETE,
            0, FALSE, cb_remove_source, s, TRUE);
  ASSERT(g_object_get_data(G_OBJECT(mi), "quickplot-source") == NULL);
  g_object_set_data(G_OBJECT(mi), "quickplot-source", s);
  gtk_widget_set_tooltip_text(mi, "Remove this buffer, its channels,"
      " and all its plots");
}


void add_source_buffer_remove_menus(struct qp_source *s)
{
  struct qp_qp *qp;
  for(qp=(struct qp_qp*)qp_sllist_begin(app->qps);
      qp; qp=(struct qp_qp*)qp_sllist_next(app->qps))
    if(qp->window)
      add_source_buffer_remove_menu(qp, s);
}


static inline
void create_button(GtkWidget *hbox, GtkAccelGroup *accelGroup,
       const char *label, guint key,
       void (*callback)(GtkWidget*, void*),
       gpointer data)
{
  GtkWidget *b = gtk_button_new_with_mnemonic(label);
  gtk_box_pack_start(GTK_BOX(hbox), b, FALSE, TRUE, 0);
  gtk_widget_add_accelerator(b, "activate", accelGroup, key,
			     (GdkModifierType) 0x0,
			     GTK_ACCEL_VISIBLE);
  gtk_widget_show(b);
  g_signal_connect(G_OBJECT(b),"clicked", G_CALLBACK(callback), data);
}


qp_qp_t qp_qp_window(struct qp_qp *qp, const char *title)
{
  GtkWidget *vbox;
  GtkAccelGroup *accelGroup;

  qp = qp_qp_check(qp);
  if(qp->window)
    /* if there is a main window already we make a
     * another qp. */
    qp = qp_qp_create();

  {
    int ret = 0;
    if(!app->is_gtk_init)
      ret = qp_app_init(NULL, NULL);
    ASSERT(ret != -1);
    if(ret == 1)
      return NULL; /* failure */
  }


  qp->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  ASSERT(qp->window);

  qp->border = app->op_border;
  gtk_window_set_decorated(GTK_WINDOW(qp->window), qp->border);
  

  qp->shape = app->op_shape;
  qp->x11_draw = app->op_x11_draw;

  gtk_container_set_border_width(GTK_CONTAINER(qp->window), 0);

  ++main_window_create_count;
  ++(app->main_window_count);
  ASSERT(app->main_window_count > 0);


  {
#define STR_LEN  128
    char str[STR_LEN];

    if(!title || !title[0])
    {
      if(qp_sllist_length(app->sources))
      {
        size_t len = STR_LEN, p = 0;
        struct qp_source *s;
        s = (struct qp_source *) qp_sllist_begin(app->sources);
        p += snprintf(str+p, len, "Quickplot: %s", s->name);
        len -= p;
        for(s=(struct qp_source *) qp_sllist_next(app->sources);
            s && p < STR_LEN && len;
            s=(struct qp_source *) qp_sllist_next(app->sources))
        {
          p += snprintf(str+p, len, " %s", s->name);
          len -= p;
        }
        if(len == 0)
          snprintf(str+(STR_LEN-5), 5, " ...");
      }
      else
        snprintf(str, 128, "Quickplot");
      title = str;
    }
#undef STR_LEN

    if(main_window_create_count > 1)
    {
      size_t len = strlen(title) + 10;
      char *t = (char *) qp_malloc(len);
      snprintf(t, len, "[%d] %s", main_window_create_count, title);
      gtk_window_set_title(GTK_WINDOW(qp->window), t);
      free(t);
    }
    else
      gtk_window_set_title(GTK_WINDOW(qp->window), title);
  }

  gtk_widget_set_events(qp->window, gtk_widget_get_events(qp->window));


  gtk_window_set_default_size(GTK_WINDOW(qp->window), 800, 700);
  gtk_window_set_icon(GTK_WINDOW(qp->window),
		      gdk_pixbuf_new_from_xpm_data(quickplot_icon));
  
  accelGroup = gtk_accel_group_new();
  gtk_window_add_accel_group(GTK_WINDOW(qp->window), accelGroup);


  vbox = gtk_vbox_new(FALSE, 0);
  ASSERT(vbox);

  gtk_container_add(GTK_CONTAINER(qp->window), vbox);
  {
    /*********************************************************************
     *                           top menu bar
     *********************************************************************/
    GtkWidget *menubar;
    qp->menubar =
    menubar = gtk_menu_bar_new();
    gtk_menu_bar_set_pack_direction(GTK_MENU_BAR(menubar),
        GTK_PACK_DIRECTION_LTR);
    gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);
    {
      GtkWidget *menu;
      /*******************************************************************
       *                   File menu
       *******************************************************************/
      qp->file_menu =
      menu = create_menu(menubar, accelGroup, "File");
      create_menu_item(menu, "_Open File ...", NULL, GTK_STOCK_OPEN,
          GDK_KEY_O, TRUE, cb_open_file, qp, TRUE);
      create_menu_item(menu, "_New Graph Tab", NULL, GTK_STOCK_NEW,
          GDK_KEY_N, TRUE, cb_new_graph_tab, qp, TRUE);
      create_menu_item(menu, "New _Window (Empty)", imgNewWindow, NULL,
          GDK_KEY_W, TRUE, cb_new_window, NULL, TRUE);
      create_menu_item(menu, "_Copy Window", imgCopyWindow, NULL,
          GDK_KEY_C, TRUE, cb_copy_window, qp, TRUE);
      qp->delete_window_menu_item =
      create_menu_item(menu, "_Delete Window", imgDeleteWindow, NULL,
          GDK_KEY_D, TRUE, cb_delete_window, qp, app->main_window_count != 1);
      if(app->main_window_count == 2)
      {
        /* We must make the other qp->delete_window_menu_item
         * have the correct sensitive state. */
        struct qp_qp *oqp;
        ASSERT(qp_sllist_length(app->qps) > 0);
        for(oqp=(struct qp_qp *)qp_sllist_begin(app->qps);
          oqp; oqp=(struct qp_qp *)qp_sllist_next(app->qps))
          if(qp->window)
          {
            gtk_widget_set_sensitive(oqp->delete_window_menu_item, TRUE);
            break;
          }
          ASSERT(oqp);
      }
      create_menu_item(menu, "Save PNG _Image File", imgSaveImage, NULL,
          GDK_KEY_I, TRUE, cb_save_png_image_file, qp, TRUE);
      create_menu_item_seperator(menu);
      create_menu_item(menu, "_Quit", NULL, GTK_STOCK_QUIT,
          GDK_KEY_Q, TRUE, cb_quit, qp->window, TRUE);
      create_menu_item_seperator(menu);
      create_menu_item_seperator(menu);
      {
        struct qp_source *s;
        for(s=(struct qp_source*)qp_sllist_begin(app->sources);
            s;s=(struct qp_source*)qp_sllist_next(app->sources))
          add_source_buffer_remove_menu(qp, s);
      }

      /*******************************************************************
       *                   View menu
       *******************************************************************/
      menu = create_menu(menubar, accelGroup, "View");
      qp->view_menubar =
      create_check_menu_item(menu, "_Menu Bar", GDK_KEY_M,
          app->op_menubar, cb_view_menubar, qp);
      qp->view_buttonbar =
      create_check_menu_item(menu, "_Button Bar", GDK_KEY_B,
          app->op_buttons, cb_view_buttonbar, qp);
      qp->view_graph_tabs =
      create_check_menu_item(menu, "Graph _Tabs", GDK_KEY_T,
          app->op_tabs, cb_view_graph_tabs, qp);
      qp->view_statusbar =
      create_check_menu_item(menu, "_Status Bar", GDK_KEY_S,
          app->op_statusbar, cb_view_statusbar, qp);

      create_menu_item_seperator(menu);

      qp->view_cairo_draw =
        create_check_menu_item(menu, "D_raw with Cairo", GDK_KEY_R,
          !qp->x11_draw, cb_view_cairo_draw, qp);
      qp->view_border =
        create_check_menu_item(menu, "Window Border (_Frame)", GDK_KEY_F,
          qp->border, cb_view_border, qp);
      gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(qp->view_border),
          gtk_window_get_decorated(GTK_WINDOW(qp->window)));
      qp->view_fullscreen =
      create_check_menu_item(menu, "F_ull Screen", GDK_KEY_U,
          (app->op_maximize == 2), cb_view_fullscreen, qp);
      qp->view_shape =
      create_check_menu_item(menu, "Shape _X11 Extension", GDK_KEY_X,
          qp->shape, cb_view_shape, qp);
      create_menu_item(menu, "_zoom Out", NULL, NULL,
          0, TRUE, cb_zoom_out, qp, TRUE);
      create_menu_item(menu, "_Zoom Out All", NULL, NULL,
          GDK_KEY_Z, TRUE, cb_zoom_out_all, qp, TRUE);
      
      create_menu_item_seperator(menu);
      
      create_check_menu_item(menu, "_Graph Configure", GDK_KEY_G,
          FALSE, dummy_callback, (void *) 0x050);
      create_check_menu_item(menu, "_Plot Lists", GDK_KEY_P,
          FALSE, dummy_callback, (void *) 15);
      /*******************************************************************
       *                   Help menu
       *******************************************************************/
      menu = create_menu(menubar, accelGroup, "Help");
      create_menu_item(menu, "_About", NULL, GTK_STOCK_ABOUT,
          GDK_KEY_A, TRUE, cb_about, NULL, TRUE);
      create_menu_item(menu, "_Help", NULL, GTK_STOCK_HELP,
          GDK_KEY_H, TRUE, cb_help, NULL, TRUE);
    }
    if(app->op_menubar)
      gtk_widget_show(menubar);  

    /*******************************************************************
     *                   Button Bar
     *******************************************************************/
    qp->buttonbar = gtk_hbox_new(FALSE, 2);
    gtk_box_pack_start(GTK_BOX(vbox), qp->buttonbar, FALSE, FALSE, 0);
    
    create_button(qp->buttonbar, accelGroup, "_Open File ...",
        GDK_KEY_O, cb_open_file, qp);
    create_button(qp->buttonbar, accelGroup, "_New Graph Tab ...",
        GDK_KEY_N, cb_new_graph_tab, qp);
    create_button(qp->buttonbar, accelGroup, "Show Confi_g ...",
        GDK_KEY_G, dummy_callback, (void *) 0x3000);
     create_button(qp->buttonbar, accelGroup, "Save PNG _Image ...",
        GDK_KEY_I, cb_save_png_image_file, qp);

    if(app->op_buttons)
	gtk_widget_show(qp->buttonbar);

    /*******************************************************************
     *                   Graph Tabs
     *******************************************************************/
    qp->notebook = gtk_notebook_new();
    
    gtk_notebook_set_show_border(GTK_NOTEBOOK(qp->notebook), FALSE);

    gtk_box_pack_start(GTK_BOX(vbox), qp->notebook, TRUE, TRUE, 0);
    g_object_set(G_OBJECT(qp->notebook), "scrollable", TRUE, NULL);
      
    if(!app->op_tabs)
      gtk_notebook_set_show_tabs(GTK_NOTEBOOK(qp->notebook), FALSE);

    /* We need at least one graph tab to start with. */
    qp->current_graph = qp_graph_create(qp, NULL);

    gtk_widget_show(qp->notebook);

    /*******************************************************************
     *                   Status Bar
     *******************************************************************/
    qp->statusbar = gtk_hbox_new(FALSE, 2);
    gtk_box_pack_start(GTK_BOX(vbox), qp->statusbar, FALSE, FALSE, 0);

    qp->status_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(qp->statusbar), qp->status_entry,
        TRUE, TRUE, 0);
    {
      PangoFontDescription *pfd;
      pfd = pango_font_description_from_string("Monospace Bold 11");
      if(pfd)
      {
        gtk_widget_override_font(qp->status_entry, pfd);
        pango_font_description_free(pfd);
      }
    }
    gtk_entry_set_text(GTK_ENTRY(qp->status_entry), "status bar");
    //g_object_set_property(G_OBJECT(qp->status_entry), "editable", FALSE);
    gtk_widget_show(qp->status_entry);

    if(app->op_statusbar)
	gtk_widget_show(qp->statusbar);
    
    /*******************************************************************
     *******************************************************************/
  }

  g_signal_connect(G_OBJECT(qp->window), "key-press-event",
      G_CALLBACK(ecb_key_press), qp);

  g_signal_connect(G_OBJECT(qp->window), "key-release-event",
      G_CALLBACK(ecb_key_release), qp);

  g_signal_connect(G_OBJECT(qp->window), "delete_event",
      G_CALLBACK(ecb_close), qp);

  g_signal_connect(G_OBJECT(qp->notebook), "switch-page",
      G_CALLBACK(cb_switch_page), NULL);


 
  if(!app->grabCursor)
  {
    GdkPixbuf *pixbuf;

    pixbuf = gdk_pixbuf_new_from_xpm_data(imgGrabCursor);
    ASSERT(pixbuf);
    app->grabCursor = gdk_cursor_new_from_pixbuf(
        gdk_display_get_default(), pixbuf, 12, 16);
    ASSERT(app->grabCursor);
    
    pixbuf = gdk_pixbuf_new_from_xpm_data(imgHoldCursor);
    ASSERT(pixbuf);
    app->holdCursor = gdk_cursor_new_from_pixbuf(
        gdk_display_get_default(), pixbuf, 12, 16);
    ASSERT(app->holdCursor);
    
    //app->grabCursor = gdk_cursor_new(GDK_HAND2);
    //app->holdCursor = gdk_cursor_new(GDK_HAND1);
    
    app->pickCursor = gdk_cursor_new(GDK_CROSSHAIR);
    app->zoomCursor = gdk_cursor_new(GDK_DRAFT_LARGE);
    app->waitCursor = gdk_cursor_new(GDK_WATCH);
  }

  qp->last_shape_region = NULL;
 

  if(app->op_maximize == 1)
  {
    gtk_window_maximize(GTK_WINDOW(qp->window));
  }
  else if(app->op_maximize == 2)
  {
    gtk_window_fullscreen(GTK_WINDOW(qp->window));
  }

  gtk_widget_show(vbox);
  gtk_widget_show(qp->window);

  DEBUG("\n");

  return qp; /* success */
}

void qp_qp_copy(struct qp_qp *old_qp, struct qp_qp *new_qp)
{
  /* TODO: write more code here */
}

