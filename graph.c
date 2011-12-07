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

#include <stdint.h>
#include <gtk/gtk.h>

#include "quickplot.h"

#include "config.h"
#include "debug.h"
#include "list.h"
#include "channel.h"
#include "channel_double.h"
#include "callbacks.h"
#include "qp.h"
#include "plot.h"
#include "zoom.h"
#include "color_gen.h"

static
size_t graph_create_count = 0;


/* drawing area minimal size restriction are
 * required so that the graph drawing algorithms do not fail. */
const gint EDGE_BUF_MIN = 10;



/* returns a molloc() allocated string */
static inline
char *unique_name(const char *name)
{
  char buf[32];
  char *test_name = NULL;
  size_t len = 0;
  int i = 1;

  ++graph_create_count;

  if(!name || !name[0])
  {
    snprintf(buf, 32, "graph %zu", graph_create_count);
    name = buf;
  }

  test_name = (char*) (name= qp_strdup(name));

  while(1)
  {
    struct qp_qp *qp;
    struct qp_graph *g = NULL;

    /* We compare with the name of all graphs in all
     * qp main windows. */
    for(qp=(struct qp_qp*)qp_sllist_begin(app->qps);
        qp; qp=(struct qp_qp*)qp_sllist_next(app->qps))
    {
      struct qp_sllist *graphs;
      graphs = qp->graphs;
      for(g=(struct qp_graph*)qp_sllist_begin(graphs); g;
        g=(struct qp_graph*)qp_sllist_next(graphs))
        if(strcmp(test_name, g->name) == 0)
          break;
      if(g)
        break;
    }

    if(g)
    {
      if(test_name == name)
        test_name = (char*) qp_malloc(len = (strlen(name)+16));
      snprintf(test_name, len, "%s[%d]", name, ++i);
    }
    else
    {
      if(test_name != name)
        free((char*) name);
      return test_name;
    }
  }
}

void add_graph_close_button(struct qp_graph *gr)
{
  GtkWidget *close_button, *image;
  ASSERT(gr->close_button == NULL);

  gr->close_button = (close_button = gtk_button_new());
  gtk_widget_set_size_request(close_button, 5, 5);

  image =
    gtk_image_new_from_stock(GTK_STOCK_CLOSE, GTK_ICON_SIZE_MENU);
  gtk_button_set_image(GTK_BUTTON(close_button), image);
  gtk_button_set_relief(GTK_BUTTON(close_button), GTK_RELIEF_NONE);
  gtk_widget_show(image);

  gtk_box_pack_start(GTK_BOX(gr->tab_label_hbox), close_button,
	FALSE, FALSE, 0);
  g_signal_connect(G_OBJECT(close_button), "clicked",
	G_CALLBACK(cb_close_tab), gr);

  gtk_widget_set_name(close_button, "tab_close_button");
  gtk_widget_set_tooltip_text(close_button,"Close tab");
  gtk_widget_show(close_button);
}


qp_graph_t qp_graph_create(qp_qp_t qp, const char *name)
{
  struct qp_graph *gr;
  ASSERT(qp);
  ASSERT(qp->graphs);
  ASSERT(qp->window);
  /* graphs do not exist unless they are
   * in a qp_qp with a gtk window. */
  if(!qp || !qp->window)
    return NULL;

  gr = (struct qp_graph *) qp_malloc(sizeof(*gr));
  gr->name = unique_name(name);
  gr->color_gen = qp_color_gen_create();
  gr->zoom_level = 0;
  gr->plots = qp_sllist_create(NULL);
  qp_sllist_append(qp->graphs, gr);
  gr->qp = qp;
  gr->pangolayout = NULL;
  gr->grid_font = qp_strdup(app->op_grid_font);

  gr->drawing_area = NULL;
  gr->close_button = NULL;
  gr->pixbuf_surface = NULL;


  gr->same_x_scale = 1;
  gr->same_y_scale = 1;
  gr->same_x_limits = 1;
  gr->same_y_limits = 1;
  gr->value_mode = (2<<2) | 1;
  gr->draw_value_pick = 0;


  /* This is the initial zoom in normalized units */
  /* qp_zoom_create(xscale, xshift, yscale, yshift) */
  gr->z = qp_zoom_create(0.95,0.025,0.92,0.04);

  memcpy(&gr->background_color, &app->op_background_color,
      sizeof(gr->background_color));
  memcpy(&gr->grid_line_color, &app->op_grid_line_color,
      sizeof(gr->grid_line_color));
  memcpy(&gr->grid_text_color, &app->op_grid_text_color,
      sizeof(gr->grid_text_color));

  gr->bg_alpha_preshape = gr->background_color.a;

  if(gr->qp->shape)
  {
    /* must be like in cb_view_shape() in callbacks.c */
    if(gr->background_color.a >= 0.5)
      gr->background_color.a = 0.4;
  }

  gr->show_grid = app->op_grid;
  gr->grid_numbers = app->op_grid_numbers;
  gr->grid_x_space = app->op_grid_x_space;
  gr->grid_y_space = app->op_grid_y_space;
  gr->grid_line_width = app->op_grid_line_width;
  gr->grid_on_top = app->op_grid_on_top;
  gr->grab_x = 0;
  gr->grab_y = 0;

  if(qp->x11_draw)
  {
    gr->x11 = qp_malloc(sizeof(*(gr->x11)));
    gr->x11->gc = 0;
    gr->x11->pixmap = 0;
    gr->x11->dsp = 0;
    gr->x11->background = 0;
    gr->x11->background_set = 0;
  }
  else
    gr->x11 = NULL;

  gr->pixbuf_width = 0;
  gr->pixbuf_height = 0;
  gr->draw_zoom_box = 0;
  gr->draw_value_pick = 0;

  /* cairo/pango set antialiasing by default
   * we need this flag for when we unset
   * antialiasing in X11 shape drawing mode. */
  gr->font_antialias_set = 1; /* boolean */

  ASSERT(app->root_window_width > 10);

  gr->drawing_area = gtk_drawing_area_new();
  gtk_widget_set_app_paintable(gr->drawing_area, TRUE);


  g_object_set_data(G_OBJECT(gr->drawing_area), "qp_graph", gr);

  gtk_widget_set_events(gr->drawing_area,
                  gtk_widget_get_events(gr->drawing_area) |
                  GDK_EXPOSURE_MASK |
                  GDK_SCROLL_MASK |
		  GDK_LEAVE_NOTIFY_MASK |
		  GDK_BUTTON_PRESS_MASK |
		  GDK_BUTTON_RELEASE_MASK |
		  GDK_POINTER_MOTION_MASK);

  /* We will do our own version of double buffering.
   * Not that it is better than GTK, but we add an additional
   * user interaction at the same time we copy image buffers.
   * Before we found this we were inadvertently triple
   * buffering the drawing of the graph. */
  gtk_widget_set_double_buffered(gr->drawing_area, FALSE);

#if 1
  {
    const GdkRGBA rgba = QP_DA_BG_RGBA;
    gtk_widget_override_background_color(gr->drawing_area,
        GTK_STATE_FLAG_NORMAL, &rgba);
  }
#endif

  gr->tab_label_hbox = gtk_hbox_new(FALSE, 3);
  g_object_set(G_OBJECT(gr->tab_label_hbox), "border-width", 0, NULL);
  
  {
    /************************************************************
     *                      Tab Label                           *
     ************************************************************/
    GtkWidget *image;
    size_t num_graphs;

    image = gtk_image_new_from_stock(GTK_STOCK_DND, GTK_ICON_SIZE_MENU);
    gtk_box_pack_start(GTK_BOX(gr->tab_label_hbox), image, FALSE, FALSE, 0);
    gtk_widget_show(image);

    gr->tab_label = gtk_label_new(gr->name);
    //gtk_label_set_selectable(GTK_LABEL(gr->tab_label), TRUE);
    //gtk_widget_set_size_request(gr->tab_label, 60, 24);
    gtk_box_pack_start(GTK_BOX(gr->tab_label_hbox), gr->tab_label,
        FALSE, FALSE, 0);
    gtk_widget_show(gr->tab_label);

    num_graphs = qp_sllist_length(qp->graphs);
    if(num_graphs > 1)
      add_graph_close_button(gr);
    if(num_graphs == 2)
    {
      ASSERT(qp_sllist_first(qp->graphs) != (void *)gr);
      add_graph_close_button((struct qp_graph*)qp_sllist_first(qp->graphs));
    }
  }
  
  gtk_notebook_append_page(GTK_NOTEBOOK(qp->notebook), gr->drawing_area,
			   gr->tab_label_hbox);


#if 0
  gtk_notebook_set_tab_label_packing(GTK_NOTEBOOK(qp->notebook),
		 gr->drawing_area,
		 FALSE, /* expand */
		 FALSE, /* fill */
		 GTK_PACK_START); // GTK_PACK_START or GTK_PACK_END
#endif
  gtk_widget_set_size_request(gr->drawing_area,
			      2*EDGE_BUF_MIN+1,
			      2*EDGE_BUF_MIN+1);

  g_signal_connect(G_OBJECT(gr->drawing_area), "draw",
      G_CALLBACK(ecb_graph_draw), gr);
  g_signal_connect(G_OBJECT(gr->drawing_area),"configure-event",
      G_CALLBACK(ecb_graph_configure), gr);
  g_signal_connect(G_OBJECT(gr->drawing_area), "button-press-event",
      G_CALLBACK(ecb_graph_button_press), gr);
  g_signal_connect(G_OBJECT(gr->drawing_area), "button-release-event",
      G_CALLBACK(ecb_graph_button_release), gr);
  g_signal_connect(G_OBJECT(gr->drawing_area), "motion-notify-event",
      G_CALLBACK(ecb_graph_pointer_motion), gr);
  g_signal_connect(G_OBJECT(gr->drawing_area), "scroll-event",
      G_CALLBACK(ecb_graph_scroll), qp);

  //g_object_set_data(G_OBJECT(gr->drawing_area), "Graph", &wrapper);

  // Give gr->drawing_area a minimum size so things do not fail.
  
  gtk_notebook_set_current_page(GTK_NOTEBOOK(qp->notebook),
      gtk_notebook_page_num(GTK_NOTEBOOK(qp->notebook), gr->drawing_area));

  gtk_widget_show(gr->tab_label_hbox);
  gtk_widget_show(gr->drawing_area);

  /* GtkNotebook refuses to switch to a page unless the child
   * widget is visible.  So this is after the show */
  gtk_notebook_set_current_page(GTK_NOTEBOOK(qp->notebook),
      gtk_notebook_get_n_pages(GTK_NOTEBOOK(qp->notebook)) - 1);


  return gr;
}

static inline
void set_x11_draw_mode(struct qp_graph *gr)
{
  struct qp_plot *p;

  ASSERT(gr->x11 == NULL);

  gr->x11 = qp_malloc(sizeof(*(gr->x11)));
  gr->x11->gc = 0;
  gr->x11->pixmap = 0;
  gr->x11->dsp = 0;
  gr->x11->background = 0;
  gr->x11->background_set = 0;

  for(p=qp_sllist_begin(gr->plots);p;p=qp_sllist_next(gr->plots))
    /* gr->x11 needs to be set so the X11 colors
     * may be made */
    qp_plot_set_x11_draw_mode(p, gr);
}

static inline
void set_cairo_draw_mode(struct qp_graph *gr)
{
  struct qp_plot *p;

  ASSERT(gr->x11);


  for(p=qp_sllist_begin(gr->plots);p;p=qp_sllist_next(gr->plots))
    /* gr->x11 needs to be set so the X11 colors
     * may be freed */
    qp_plot_set_cairo_draw_mode(p, gr);


  if(gr->x11)
  {
    if(gr->x11->gc)
      XFreeGC(gr->x11->dsp, gr->x11->gc);
    if(gr->x11->pixmap)
        XFreePixmap(gr->x11->dsp, gr->x11->pixmap);
    free(gr->x11);
    gr->x11 = NULL;
  }
}

void qp_graph_switch_draw_mode(struct qp_graph *gr)
{

  if((gr->x11 && gr->qp->x11_draw) ||
      (!gr->x11 && !gr->qp->x11_draw))
    return;

  if(gr->qp->x11_draw)
    set_x11_draw_mode(gr);
  else
    set_cairo_draw_mode(gr);

  if(gr->pixbuf_surface)
  {
    /* This gets recreated in the drawing */
    cairo_surface_destroy(gr->pixbuf_surface);
    gr->pixbuf_surface = NULL;
  }
  gr->pixbuf_needs_draw = 1;
}

void qp_graph_destroy(qp_graph_t gr)
{
  struct qp_qp *qp;
  ASSERT(gr);
  ASSERT(gr->qp);
  ASSERT(gr->qp->graphs);
  ASSERT(gr->name);
  ASSERT(gr->plots);
  if(!gr) return;

  qp = gr->qp;
  ASSERT(qp);
  ASSERT(qp->window);
 

  {
    struct qp_plot *p;
    p = qp_sllist_begin(gr->plots);
    for(; p; p = qp_sllist_next(gr->plots))
      qp_plot_destroy(p, gr);
    qp_sllist_destroy(gr->plots, 0);
  }

  /* this will remove the tab */
  gtk_widget_destroy(gr->drawing_area);
  qp_color_gen_destroy(gr->color_gen);

  free(gr->name);
  qp_sllist_remove(gr->qp->graphs, gr, 0);
 
  if(gr->pixbuf_surface)
    cairo_surface_destroy(gr->pixbuf_surface);

  if(gr->x11)
  {
    if(gr->x11->gc)
      XFreeGC(gr->x11->dsp, gr->x11->gc);
    if(gr->x11->pixmap)
        XFreePixmap(gr->x11->dsp, gr->x11->pixmap);
    free(gr->x11);
  }

  ASSERT(gr->grid_font);

  free(gr->grid_font);

  if(gr->pangolayout)
    g_object_unref(G_OBJECT(gr->pangolayout));

  free(gr);
  /* done with this graph */


  if(qp_sllist_length(qp->graphs) == 1)
  {
    gr = qp_sllist_first(qp->graphs);
    ASSERT(gr->close_button);
    gtk_widget_destroy(gr->close_button);
    gr->close_button = NULL;
  }
}

void qp_graph_zoom_out(struct qp_graph *gr, int all)
{
  if(gr->zoom_level == 0 && !gr->grab_x && !gr->grab_y)
    return;

  if(all)
  {
    /* zoom all the way out */
    if(gr->zoom_level)
      gr->pixbuf_needs_draw = 1;
    gr->zoom_level = 0;
    qp_zoom_pop_all(&(gr->z));
    gr->grab_x = gr->grab_y = 0;
    gdk_window_set_cursor(gtk_widget_get_window(gr->qp->window), app->waitCursor);
  }
  else if(gr->grab_x || gr->grab_y)
  {
    gr->grab_x = gr->grab_y = 0;
  }
  else
  {
    /* zoom out one level */
    --gr->zoom_level;
    gr->pixbuf_needs_draw = 1;
    qp_zoom_pop(&(gr->z));
    gdk_window_set_cursor(gtk_widget_get_window(gr->qp->window), app->waitCursor);
  }

  qp_qp_set_status(gr->qp);
  gtk_widget_queue_draw(gr->drawing_area);
}

