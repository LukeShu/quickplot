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
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>
#include <cairo/cairo-xlib.h>

#include "quickplot.h"

#include "config.h"
#include "debug.h"
#include "list.h"
#include "callbacks.h"
#include "channel.h"
#include "channel_double.h"
#include "qp.h"
#include "plot.h"
#include "zoom.h"


int save_x, save_y, start_x, start_y;
int mouse_num = 0, got_motion = 0;

#define L_SHIFT  (01<<0)
#define R_SHIFT  (01<<1)
#define L_CNTL   (01<<2)
#define R_CNTL   (01<<3)


/* optionally, we use the shift/cntl key press/release
 * in the zoom box adjusting */
static int got_mod_key = 0;


gboolean ecb_close(GtkWidget *w, GdkEvent *event, gpointer data)
{
  ASSERT(data);
  ASSERT(app && app->is_gtk_init);
  ASSERT(app->main_window_count > 0);

  if(app->gui_can_exit && app->main_window_count == 1)
  {
    cb_quit(NULL, data);
    return TRUE;
  }

  cb_delete_window(NULL, data);
  return TRUE; /* TRUE means event is handled */
}

void cb_delete_window(GtkWidget *w, gpointer data)
{
  struct qp_qp *qp;
  ASSERT(data);
  VASSERT(app->main_window_count > 1,
      "the delete window menu item should be deactivated if"
      " there is just one main quickplot window");
  qp = data;

  qp_qp_destroy(qp);
}

void cb_quit(GtkWidget *w, gpointer data)
{
  struct qp_qp *qp;
  ASSERT(app && app->is_gtk_init);


  while((qp = qp_sllist_last(app->qps)))
    qp_qp_destroy(qp);

  ASSERT(qp_sllist_length(app->qps) == 0);

  gtk_main_quit();
}

gboolean ecb_key_release(GtkWidget *w, GdkEvent *event, gpointer data)
{
  switch(event->key.keyval)
  {
    case GDK_KEY_Shift_L:
      got_mod_key &= ~L_SHIFT;
      break;
    case GDK_KEY_Shift_R:
      got_mod_key &= ~R_SHIFT;
      break;
    case GDK_KEY_Control_L:
      got_mod_key &= ~L_CNTL;
      break;
    case GDK_KEY_Control_R:
      got_mod_key &= ~R_CNTL;
      break;
  }
  return FALSE;
}

static inline
void toggle_all_guis(struct qp_qp *qp)
{
  int showing;

  showing =
    gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(qp->view_menubar))
    ||
    gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(qp->view_buttonbar))
    ||
    gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(qp->view_graph_tabs))
    ||
    gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(qp->view_statusbar));

  if(showing)
  {
    /* Hide them */
    if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(qp->view_menubar)))
      gtk_menu_item_activate(GTK_MENU_ITEM(qp->view_menubar));
    if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(qp->view_buttonbar)))
      gtk_menu_item_activate(GTK_MENU_ITEM(qp->view_buttonbar));
    if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(qp->view_graph_tabs)))
      gtk_menu_item_activate(GTK_MENU_ITEM(qp->view_graph_tabs));
    if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(qp->view_statusbar)))
      gtk_menu_item_activate(GTK_MENU_ITEM(qp->view_statusbar));

    return;
  }

  /* Show them */
  if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(qp->view_menubar)))
    gtk_menu_item_activate(GTK_MENU_ITEM(qp->view_menubar));
  if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(qp->view_buttonbar)))
    gtk_menu_item_activate(GTK_MENU_ITEM(qp->view_buttonbar));
  if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(qp->view_graph_tabs)))
    gtk_menu_item_activate(GTK_MENU_ITEM(qp->view_graph_tabs));
  if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(qp->view_statusbar)))
    gtk_menu_item_activate(GTK_MENU_ITEM(qp->view_statusbar));
}

gboolean ecb_key_press(GtkWidget *w, GdkEvent *event, gpointer data)
{
  struct qp_qp *qp;
  ASSERT(data);
  ASSERT(event->type == GDK_KEY_PRESS);
  qp = data;

  //DEBUG("got press keyval=0x%x\n", event->key.keyval);

  // Since the key accelerators that are associated with the menus
  // above do not work if the menu is not visible we just handle the
  // key press here.  We still need the key letter to be labeled on
  // the menu item which the accelerator does, so we setup the
  // accelerators just for the label, but otherwise the accelerators
  // could be removed since we override them here.
  /********************************************************************
   *                   MAP KEY PRESSES for main window                *
   ********************************************************************/
  // KEEP THESE KEY MAPPING CONSISTENT WITH THE MENU ITEMS ABOVE.
  // Yes, this is a not so nice hack.  And GTK+ sucks, because it
  // stops accelerators when widgets are not visible, but it beats QT
  // (with butt ugly moc preprocessor) and GTKmm (which is a wrapper
  // that makes you go under the covers to get shit done).  I prefer
  // this hack to making another abstraction layer that you put keys
  // and actions into.  This is the most straight forward method.

  switch(event->key.keyval)
    {
    case GDK_KEY_A:
    case GDK_KEY_a:
      cb_about(NULL, NULL);
      break;
    case GDK_KEY_B:
    case GDK_KEY_b:
       gtk_menu_item_activate(GTK_MENU_ITEM(qp->view_buttonbar));
      break;
    case GDK_KEY_C:
    case GDK_KEY_c:
      cb_copy_window(NULL, qp);
      break;
    case GDK_KEY_D:
    case GDK_KEY_d:
    case GDK_KEY_Escape:
      if(app->main_window_count > 1)
        ecb_close(NULL, NULL, qp);
      break;
    case GDK_KEY_E:
    case GDK_KEY_e:
      gtk_menu_item_activate(GTK_MENU_ITEM(qp->view_border));
      break;
    case GDK_KEY_F:
    case GDK_KEY_f:
      gtk_menu_item_activate(GTK_MENU_ITEM(qp->view_fullscreen));
      break;
    case GDK_KEY_G:
    case GDK_KEY_g:
      break;
    case GDK_KEY_H:
    case GDK_KEY_h:
      cb_help(NULL, NULL);
      break;
    case GDK_KEY_I:
    case GDK_KEY_i:
      cb_save_png_image_file(NULL, qp);
      break;
    case GDK_KEY_M:
    case GDK_KEY_m:
      gtk_menu_item_activate(GTK_MENU_ITEM(qp->view_menubar));
      break;
    case GDK_KEY_N:
    case GDK_KEY_n:
      cb_new_graph_tab(NULL, qp);
      break;
    case GDK_KEY_O:
    case GDK_KEY_o:
      cb_open_file(NULL, qp);
      break;
    case GDK_KEY_P:
    case GDK_KEY_p:
      break;
    case GDK_KEY_Q:
    case GDK_KEY_q:
      cb_quit(qp->window, NULL);
      break;
    case GDK_KEY_R:
    case GDK_KEY_r:
      gtk_menu_item_activate(GTK_MENU_ITEM(qp->view_cairo_draw));
      break;
    case GDK_KEY_S:
    case GDK_KEY_s:
      gtk_menu_item_activate(GTK_MENU_ITEM(qp->view_statusbar));
      break;
    case GDK_KEY_T:
    case GDK_KEY_t:
      gtk_menu_item_activate(GTK_MENU_ITEM(qp->view_graph_tabs));
      break;
    case GDK_KEY_U:
    case GDK_KEY_u:
      toggle_all_guis(qp);
      break;
    case GDK_KEY_W:
    case GDK_KEY_w:
      cb_new_window(NULL, NULL);
      break;
    case GDK_KEY_X:
    case GDK_KEY_x:
      gtk_menu_item_activate(GTK_MENU_ITEM(qp->view_shape));
      break;
    case GDK_KEY_Z:
      qp_graph_zoom_out(qp->current_graph, 1);
      break;
    case GDK_KEY_z:
      qp_graph_zoom_out(qp->current_graph, 0);
      break;
    case GDK_KEY_Shift_L:
      got_mod_key |= L_SHIFT;
      break;
    case GDK_KEY_Shift_R:
      got_mod_key |= R_SHIFT;
      break;
    case GDK_KEY_Control_L:
      got_mod_key |= L_CNTL;
      break;
    case GDK_KEY_Control_R:
      got_mod_key |= R_CNTL;
      break;
    case GDK_KEY_Left:
    case GDK_KEY_leftarrow:
      if(gtk_notebook_get_show_tabs(GTK_NOTEBOOK(qp->notebook)) ||
          gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(qp->view_buttonbar)))
        return FALSE;
      else
        gtk_notebook_prev_page(GTK_NOTEBOOK(qp->notebook));
      break;
    case GDK_KEY_Right:
    case GDK_KEY_rightarrow:
      if(gtk_notebook_get_show_tabs(GTK_NOTEBOOK(qp->notebook)) ||
          gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(qp->view_buttonbar)))
        return FALSE;
      else
        gtk_notebook_next_page(GTK_NOTEBOOK(qp->notebook));
      break;
    default:
      return FALSE; /* FALSE means the event is not handled. */
    }

  return TRUE; /* TRUE means the event is handled. */
}

void cb_zoom_out(GtkWidget *w, gpointer data)
{
  struct qp_qp *qp;
  qp = data;
  qp_graph_zoom_out(qp->current_graph, 0);
}

void cb_zoom_out_all(GtkWidget *w, gpointer data)
{
  struct qp_qp *qp;
  qp = data;
  qp_graph_zoom_out(qp->current_graph, 1);
}

void cb_view_menubar(GtkWidget *w, gpointer data)
{
  struct qp_qp *qp;
  ASSERT(data);
  qp = data;

  if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(qp->view_menubar)))
    gtk_widget_show(qp->menubar);
  else
    gtk_widget_hide(qp->menubar);
  
  gdk_window_set_cursor(gtk_widget_get_window(qp->window), app->waitCursor);
}

void cb_view_graph_tabs(GtkWidget *w, gpointer data)
{
  struct qp_qp *qp;
  ASSERT(data);
  qp = data;

  if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(qp->view_graph_tabs)))
    gtk_notebook_set_show_tabs(GTK_NOTEBOOK(qp->notebook), TRUE);
  else
    gtk_notebook_set_show_tabs(GTK_NOTEBOOK(qp->notebook), FALSE);

  gdk_window_set_cursor(gtk_widget_get_window(qp->window), app->waitCursor);
}

void cb_view_buttonbar(GtkWidget *w, gpointer data)
{
  struct qp_qp *qp;
  ASSERT(data);
  qp = data;

  if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(qp->view_buttonbar)))
    gtk_widget_show(qp->buttonbar);
  else
    gtk_widget_hide(qp->buttonbar);

  gdk_window_set_cursor(gtk_widget_get_window(qp->window), app->waitCursor);
}

void cb_view_statusbar(GtkWidget *w, gpointer data)
{
  struct qp_qp *qp;
  ASSERT(data);
  qp = data;

  if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(qp->view_statusbar)))
    gtk_widget_show(qp->statusbar);
  else
    gtk_widget_hide(qp->statusbar);
  
  gdk_window_set_cursor(gtk_widget_get_window(qp->window), app->waitCursor);
}


void cb_view_border(GtkWidget *w, gpointer data)
{
  struct qp_qp *qp;
  gboolean i;
  ASSERT(data);
  qp = data;

  if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(qp->view_border)))
  {
    gtk_window_set_decorated(GTK_WINDOW(qp->window),  qp->border = TRUE);
    i = gtk_window_get_decorated(GTK_WINDOW(qp->window));
    if(!i)
    {
      gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(qp->view_border), FALSE);
      qp->border = FALSE;
    }
  }
  else
  {
    gtk_window_set_decorated(GTK_WINDOW(qp->window), qp->border = FALSE);
    i = gtk_window_get_decorated(GTK_WINDOW(qp->window));
    if(i)
    {
      gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(qp->view_border), TRUE);
      qp->border = TRUE;
    }
  }
}

void cb_view_fullscreen(GtkWidget *w, gpointer data)
{
  struct qp_qp *qp;
  ASSERT(data);
  qp = data;

  if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(qp->view_fullscreen)))
    gtk_window_fullscreen(GTK_WINDOW(qp->window));
  else
    gtk_window_unfullscreen(GTK_WINDOW(qp->window));

  /* if we set the waitCursor but do not have a redraw happen it would
   * be bad, given the wait cursor would not change back until the
   * next redraw.  So maybe it would be better to force the redraw
   * which would do nothing if it works and will add an extra redraw
   * if the set fullscreen failed and the wait cursor will not be stuck.
   *
   * tracking the window-state-event may not work, because the
   * wait cursor will get set too late to show.
   *
   * If will work in all cases and in the off chance that
   * gtk_window_*fullscreen() fails the results not too bad.
   * */
  gtk_widget_queue_draw(qp->current_graph->drawing_area);
  qp->current_graph->pixbuf_needs_draw = 1;
  gdk_window_set_cursor(gtk_widget_get_window(qp->window), app->waitCursor);
}

static __thread int cairo_draw_ignore_event = 0;

void cb_view_cairo_draw(GtkWidget *w, gpointer data)
{
  struct qp_qp *qp;

  if(cairo_draw_ignore_event)
  {
    //DEBUG("ignoring\n");
    return;
  }

  ASSERT(data);
  qp = data;


  if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(qp->view_cairo_draw)))
    qp->x11_draw = 0;
  else
    qp->x11_draw = 1;

  qp_graph_switch_draw_mode(qp->current_graph);
  ecb_graph_configure(NULL, NULL, qp->current_graph);

  gtk_widget_queue_draw(qp->current_graph->drawing_area);
  gdk_window_set_cursor(gtk_widget_get_window(qp->window), app->waitCursor);
}

void cb_view_shape(GtkWidget *w, gpointer data)
{
  struct qp_qp *qp;
  struct qp_graph *gr;
  ASSERT(data);
  qp = data;

  /* this must set the gr->background_color consistant 
   * with qp_graph_create() in graph.c */

  if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(qp->view_shape)))
  {
    DEBUG("shape on\n");
    /* turn on shape */

    for(gr=qp_sllist_begin(qp->graphs);gr;gr=qp_sllist_next(qp->graphs))
    {
      /* Full redraw needed to unset anti-aliasing */
      gr->pixbuf_needs_draw = 1;
      if(gr->background_color.a >= 0.5)
        gr->background_color.a = 0.4;
    }

    qp->shape = 1;
    gdk_window_set_cursor(gtk_widget_get_window(qp->window),app->waitCursor);
  }
  else
  {
    DEBUG("shape off\n");
    /* turn off shape */

    for(gr=qp_sllist_begin(qp->graphs);gr;gr=qp_sllist_next(qp->graphs))
    {
      /* Full redraw needed to set anti-aliasing */
      gr->pixbuf_needs_draw = 1;
      if(gr->background_color.a != gr->bg_alpha_preshape)
        gr->background_color.a  = gr->bg_alpha_preshape;
    }

    qp->shape = 0;

    /* This should remove the current shape thingy */
    /* by applying a NULL shape_region */
    gtk_widget_shape_combine_region(qp->window, NULL);

    if(qp->last_shape_region)
    {
      cairo_region_destroy(qp->last_shape_region);
      qp->last_shape_region = NULL;
      /* now it is NULL in our record of it */
    }

    if(qp->current_graph->pixbuf_needs_draw)
      gdk_window_set_cursor(gtk_widget_get_window(qp->window),app->waitCursor);
   }

  
  gtk_widget_queue_draw(qp->current_graph->drawing_area);
}

void cb_close_tab(GtkWidget *w, gpointer data)
{
  ASSERT(data);
  qp_graph_destroy((struct qp_graph *) data);
}

gboolean ecb_graph_draw(GtkWidget *w, cairo_t *cr, gpointer data)
{
  struct qp_graph *gr;
  ASSERT(data);
  gr = data;

  qp_graph_draw(gr, cr); 

  return TRUE; /* TRUE means the event is handled. */
}

void cb_about(GtkWidget *w, gpointer data)
{
  qp_launch_browser("about.html");

  if(data)
  {
    printf("Quickplot version " VERSION "\n"
      "The Quickplot home-page is at " PACKAGE_URL "\n"
      "This was compiled on: " __DATE__ " at the time: " __TIME__ "\n"
      "Quickplot is free software, distributed under the terms of the\n"
      "GNU General Public License (version 3 or higher).\n"
#ifdef QP_DEBUG
      "This Quickplot installation was built with extra debuging code.\n"
      "You do not want that unless you are writing Quickplot code.\n"
#endif
      );
    exit(0);
  }
}

void cb_help(GtkWidget *w, gpointer data)
{
  qp_launch_browser("help.html");

  if(data)
  {
    printf("Quickplot is a fast interactive 2D plotter.\n"
        "\n"
        "Quickplot uses html and man pages files for its reference\n"
        "documentation.  If you're not seeing a browser now with the\n"
        "Quickplot Help document loaded than you may not be using a\n"
        "windowing system or something else went wrong.\n"
        "\n"
        "The Quickplot text help document can be printed to standard\n"
        "output by running `quickplot --print-help'.\n"
        "\n"
        "The Quickplot home-page is at http://quickplot.sourceforge.net\n");
    exit(1);
  }
}

gboolean ecb_graph_configure(GtkWidget *wdgt, GdkEvent *event, gpointer data)
{
  struct qp_graph *gr;
  GtkAllocation allocation;
  int w1, w2, h1, h2, h, w;
  int old_xscale, old_yscale;
  ASSERT(data);
  gr = data;
  ASSERT(gr->drawing_area);
  gtk_widget_get_allocation(gr->drawing_area, &allocation);

  w1 = 2*allocation.width;
  w2 = 1.5*app->root_window_width;
  h1 = 2*allocation.height;
  h2 = 1.5*app->root_window_height;
  w = (w1 > w2)?w1:w2;
  h = (h1 > h2)?h1:h2;

  if(!gr->pixbuf_surface || w > gr->pixbuf_width || h > gr->pixbuf_height)
  {

    if(gr->x11)
    {
      /* Drawing with X11 and Cairo */

      unsigned int depth;
      Display *dsp;
      Window win;
      int screen;

      if(!gr->x11->dsp)
        dsp = (gr->x11->dsp = gdk_x11_get_default_xdisplay());
      else
        dsp = gr->x11->dsp;

      screen = DefaultScreen(gr->x11->dsp);
      depth = DefaultDepth(dsp, screen);
   
      ASSERT(depth > 7);

      win = gdk_x11_window_get_xid(gtk_widget_get_window(gr->drawing_area));

      if(gr->pixbuf_surface)
        cairo_surface_destroy(gr->pixbuf_surface);

      if(gr->x11->gc)
        XFreeGC(dsp, gr->x11->gc);

      if(gr->x11->pixmap)
        XFreePixmap(dsp, gr->x11->pixmap);

      gr->x11->pixmap =
        XCreatePixmap(dsp, win, w, h, depth);


      gr->x11->gc = XCreateGC(dsp, gr->x11->pixmap, 0, 0);
      XSetLineAttributes(dsp, gr->x11->gc, 4, LineSolid, CapButt, JoinRound);

      gr->pixbuf_surface =
        cairo_xlib_surface_create(dsp, gr->x11->pixmap,
            DefaultVisual(dsp, screen), w, h);

      //DEBUG("Using X11 Pixmap\n");

    }
    else
    {
      /* Drawing just with Cairo */

      if(gr->pixbuf_surface)
        cairo_surface_destroy(gr->pixbuf_surface);

      gr->pixbuf_surface =
        cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
    }

    /* The drawing_area widget fits inside the pixbuf */


                                ////////pixbuf////////
                                //                  //
                                //    **********    //
                                //    *        *    //
    gr->pixbuf_width = w;       //    * widget *    //
    gr->pixbuf_height = h;      //    *        *    //
                                //    **********    //
                                //                  //
                                //////////////////////
  }

  gr->pixbuf_x = (gr->pixbuf_width - allocation.width)/2;
  gr->pixbuf_y = (gr->pixbuf_height - allocation.height)/2;

  gr->pixbuf_needs_draw = 1;

  // Too bad this does not work
  gdk_window_set_cursor(gtk_widget_get_window(gr->qp->window),
                  app->waitCursor);

  /* plot_val = scale*(zoomed plot val) + shift */

  /* if we have any grab shifting we scale that to work
   * with the new tranformation */

  old_xscale = gr->xscale;
  old_yscale = gr->yscale;
 
  gr->xscale =  allocation.width;
  gr->yscale =  - allocation.height;

  gr->xshift = 0;
  gr->yshift = allocation.height;

  /* multiply out the new scale to the grab */
  if(gr->grab_x)
    gr->grab_x *= ((double)gr->xscale)/((double)old_xscale);
  if(gr->grab_y)
    gr->grab_y *= ((double)gr->yscale)/((double)old_yscale);

  gr->waiting_to_resize_draw = 1;



  if(ABSVAL(gr->grab_x) > gr->pixbuf_x ||
          ABSVAL(gr->grab_y) > gr->pixbuf_y)
  {
    qp_zoom_push_shift(&(gr->z), - gr->grab_x/gr->xscale,
                                 - gr->grab_y/gr->yscale);
    ++gr->zoom_level;
    gr->grab_x = 0;
    gr->grab_y = 0;
  }

  gr->qp->pointer_x = -1;
  gr->qp->pointer_y = -1;


  //DEBUG("graph named: \"%s\" surface=%p\n", gr->name, gr->surface);
  return TRUE; /* TRUE means the event is handled. */
}

/* We use this to zoom in and out at the pointer */
gboolean ecb_graph_scroll(GtkWidget *w, GdkEvent *event, gpointer data)
{
#ifdef QP_DEBUG
  const char *dir[] = { "up", "down", "left", "right"};
  DEBUG("x=%g y=%g direction=%s time=%u\n",
      event->scroll.x, event->scroll.y,
      dir[((int) (event->scroll.direction)) % 4],
      event->scroll.time);

#endif

  return TRUE;
}

gboolean cb_switch_page(GtkNotebook *notebook, GtkWidget *page,
    guint page_num, gpointer data)
{
  struct qp_graph *gr;
  ASSERT(page);
  gr = g_object_get_data(G_OBJECT(page), "qp_graph");
  ASSERT(gr);
  
  /* gtk_notebook_get_current_page(GTK_NOTEBOOK(qp->notebook)) is broken
   * so we catch this event to get the current graph that is showing here. */
  gr->qp->current_graph = gr;

  //WARN("page_num=%d graph named=\"%s\"\n", page_num, gr->name);
 
  qp_qp_set_status(gr->qp);

  if((gtk_check_menu_item_get_active(
          GTK_CHECK_MENU_ITEM(gr->qp->view_cairo_draw)) &&
      gr->x11) ||
    (!gtk_check_menu_item_get_active(
          GTK_CHECK_MENU_ITEM(gr->qp->view_cairo_draw)) &&
      !gr->x11) )
  {
    cairo_draw_ignore_event = 1;
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gr->qp->view_cairo_draw),
        (gr->x11)?FALSE:TRUE);
    cairo_draw_ignore_event = 0;
  }

  // redraw queuing is not needed
  //gtk_widget_queue_draw(gr->drawing_area);

  /* a new graph will use the draw mode of this graph */
  gr->qp->x11_draw = (gr->x11)?1:0;

  if(gr->qp->shape)
    gdk_window_set_cursor(gtk_widget_get_window(gr->qp->window),app->waitCursor);

  return TRUE;
}

gboolean ecb_graph_button_press(GtkWidget *w, GdkEvent *event, gpointer data)
{
  struct qp_graph *gr;
  ASSERT(data);
  ASSERT(event->type == GDK_BUTTON_PRESS ||
      event->type == GDK_2BUTTON_PRESS ||
      event->type == GDK_3BUTTON_PRESS);

  /* We can get a button press without a release when
   * we include the double and triple press events.
   * Double presses can come without a single press
   * before. */

  gr = data;
  
  //ERROR("graph named: \"%s\" [type=%d] mouse at=(%g,%g) button=%d\n",
  //    gr->name, event->type,
  //    event->button.x, event->button.y, event->button.button);

  if(event->button.button == GRAB_BUTTON ||
    event->button.button == PICK_BUTTON ||
    event->button.button == ZOOM_BUTTON)
  {
    if(mouse_num)
    {
      /* we do not do multi-button stuff
       * We just take care of the first button press
       * hoping we will get a button release event that resets
       * mouse_num to 0. */
      return TRUE;
    }
    start_x = save_x = (int) event->button.x;
    start_y = save_y = (int) event->button.y;
    mouse_num = event->button.button;
    got_motion = 0;
    /* we use the shift in the zoom box drawing to
     * shift the zoom box instead of just pulling it.
     * We ignore all shift key presses before the box
     * is started from this button press. */
    got_mod_key = 0;
  }
  else
    return FALSE;

  switch(mouse_num)
  {
    case GRAB_BUTTON:
      gdk_window_set_cursor(gtk_widget_get_window(gr->drawing_area), app->grabCursor); 
      break;
    case PICK_BUTTON:

      break;
    case ZOOM_BUTTON:
      gdk_window_set_cursor(gtk_widget_get_window(gr->drawing_area), app->zoomCursor);
      break;
  }

  return TRUE; /* TRUE means the event is handled. */
}

/* sort it so that y >= x */
static inline
void Order(int *x, int *y)
{
  if(*x > *y)
  {
    int i;
    i = *x;
    *x = *y;
    *y = i;
  }
}

gboolean ecb_graph_button_release(GtkWidget *w, GdkEvent *event,
    gpointer data)
{
  struct qp_graph *gr;
  double width, height;
  ASSERT(data);
  ASSERT(event->type == GDK_BUTTON_RELEASE);
  gr = data;
  //DEBUG("graph named: \"%s\"\n", gr->name);

  gr->qp->pointer_x = event->button.x;
  gr->qp->pointer_y = event->button.y;
  
  if(mouse_num != GRAB_BUTTON)
    qp_qp_set_status(gr->qp);

  if(event->button.button != GRAB_BUTTON &&
    event->button.button != PICK_BUTTON &&
    event->button.button != ZOOM_BUTTON)
    return FALSE;

  /* in order to keep things working in the most cases we
   * accept any button release event to finish the mouse
   * press and pointer/motion action. */

  // this test shows the the pointer can move
  // between motion and this release event
  // We just ignore this motion since it would
  // just make things looks jiggley (bad).
  //ASSERT(save_x == (int) event->button.x);
  //ASSERT(save_y == (int) event->button.y);

  width = gtk_widget_get_allocated_width(gr->drawing_area);
  height = gtk_widget_get_allocated_height(gr->drawing_area);

  switch(mouse_num)
  {
    case GRAB_BUTTON:
      gdk_window_set_cursor(gtk_widget_get_window(gr->drawing_area), NULL);

      if(ABSVAL(gr->grab_x) > gr->pixbuf_x ||
          ABSVAL(gr->grab_y) > gr->pixbuf_y)
      {
        qp_zoom_push_shift(&(gr->z), - gr->grab_x/gr->xscale,
                                     - gr->grab_y/gr->yscale);
        ++gr->zoom_level;
        gr->grab_x = 0;
        gr->grab_y = 0;
        gr->pixbuf_needs_draw = 1;
        gdk_window_set_cursor(gtk_widget_get_window(gr->qp->window),
            app->waitCursor);
        gtk_widget_queue_draw(gr->drawing_area);
      }
      else
        /* In case there is mouse motion */
        qp_qp_set_status(gr->qp);
      break;
    case PICK_BUTTON:

      break;
    case ZOOM_BUTTON:
      {
        int queue_draw = 0;
        int starting_zoom_level;
        starting_zoom_level = gr->zoom_level;

        gdk_window_set_cursor(gtk_widget_get_window(gr->drawing_area), NULL);
        if(gr->draw_zoom_box)
        {
          gr->draw_zoom_box = 0;
          queue_draw = 1;
        }

        /* order the x and y values so save >= start */
        Order(&start_x, &save_x);
        Order(&start_y, &save_y);
 
        /* We will zoom in under these conditions
         * But we will save the grab to a zoom first
         * if we zoom in at all */
        if((start_x >= 0 && save_x < width &&
            start_y >= 0 && start_y < height) ||
            save_x - start_x < 2 ||
            save_y - start_y < 2)
        {
          if(gr->grab_x || gr->grab_y)
          {
            qp_zoom_push_shift(&(gr->z), - gr->grab_x/gr->xscale,
                                         - gr->grab_y/gr->yscale);
            ++gr->zoom_level;
            gr->grab_x = 0;
            gr->grab_y = 0;
          }
        }
         
      /* We have a zoom box defined by start_x, start_y,
       * and save_x, save_y. */


      if((save_x - start_x)*(save_y - start_y) < 9)
      {
        /* we just shrink the plots a little */
        /* We make a zoom level that zooms out */
        qp_zoom_push(&(gr->z), 0.8, 0.1, 0.8, 0.1); 
        ++gr->zoom_level;
      }
      else if(start_x >= 0 && save_x < width &&
          start_y >= 0 && save_y < height)
      {
        /* We make a new zoom from the zoom box */
        double xmin, ymin, xmax, ymax;
        xmin = start_x;
        xmax = save_x;
        ymin = height - save_y;
        ymax = height - start_y;
        xmin /= width;
        xmax /= width;
        ymin /= height;
        ymax /= height;
        qp_zoom_push_mm(&(gr->z), xmin, ymin, xmax, ymax);
        ++gr->zoom_level;
      }
      else if((start_x >= 0 && save_x < width) ||
        (start_y >= 0 && save_y < height))
      {
        if(gr->grab_x || gr->grab_y)
        {
          gr->grab_x = gr->grab_y = 0;
          queue_draw = 1;
        }
        else if(gr->zoom_level)
        {
          /* zoom out one level */
          qp_zoom_pop(&(gr->z));
          --gr->zoom_level;
        }
      }
      else
      {
        if(gr->zoom_level)
        {
          /* zoom out all levels */
          qp_zoom_pop_all(&(gr->z));
          gr->zoom_level = 0;
        }
        if(gr->grab_x || gr->grab_y)
        {
          gr->grab_x = gr->grab_y = 0;
          queue_draw = 1;
        }
      }

      if(gr->zoom_level != starting_zoom_level)
      {
        gdk_window_set_cursor(gtk_widget_get_window(gr->qp->window),
            app->waitCursor);
        queue_draw = 1;
        gr->pixbuf_needs_draw = 1;
      }

      if(queue_draw)
      {
        qp_qp_set_status(gr->qp);
        gtk_widget_queue_draw(gr->drawing_area);
      }

      break;
    }
  }


  /* reset the mouse_num */
  mouse_num = 0;

  return TRUE; /* TRUE means the event is handled. */
}

gboolean ecb_graph_pointer_motion(GtkWidget *w, GdkEvent *event, gpointer data)
{
  struct qp_graph *gr;
  struct qp_qp *qp;
  ASSERT(data);
  ASSERT(event->type == GDK_MOTION_NOTIFY);
  gr = data;
  qp = gr->qp;

  gr->qp->pointer_x = event->motion.x;
  gr->qp->pointer_y = event->motion.y;
  
  if(mouse_num != GRAB_BUTTON)
    qp_qp_set_status(gr->qp);


  if(!mouse_num) return FALSE;

  //DEBUG("graph named: \"%s\"\n", gr->name);


  switch(mouse_num)
  {
    case GRAB_BUTTON:
      if(!got_motion)
      {
        gdk_window_set_cursor(gtk_widget_get_window(gr->drawing_area),
          app->holdCursor);
      }

      //DEBUG("from (x,y) %d, %d to %d, %d\n", save_x, save_y,
        //  (int)event->motion.x, (int) event->motion.y);

      gr->grab_x += save_x - (int) event->motion.x;
      gr->grab_y += save_y - (int) event->motion.y;

      gtk_widget_queue_draw(gr->drawing_area);

      if(!got_motion)
        qp_qp_set_status(qp);
     
      break;
    case PICK_BUTTON:

      break;
    case ZOOM_BUTTON:
      {
        if(got_mod_key & (L_SHIFT | R_SHIFT))
        {
          start_x -= save_x - (int) event->motion.x;
          start_y -= save_y - (int) event->motion.y;
        }
        else if(got_mod_key & (L_CNTL | R_CNTL))
        {
          start_x += save_x - (int) event->motion.x;
          start_y += save_y - (int) event->motion.y;
        }

        if(start_x != (int) event->motion.x &&
           start_y != (int) event->motion.y)
        {
          gr->draw_zoom_box = 1;
          gr->z_x = start_x;
          gr->z_y = start_y;
          gr->z_w = (int) event->motion.x - start_x;
          gr->z_h = (int) event->motion.y - start_y;
          /* clear the old zoom box and draw a new one */
          gtk_widget_queue_draw(gr->drawing_area);
        }
        else if(gr->draw_zoom_box)
        {
          gr->draw_zoom_box = 0;
          /* clear the old zoom box and do not draw a new one */
          gtk_widget_queue_draw(gr->drawing_area);
        }
      }
      break;
  }

  save_x = (int) event->motion.x;
  save_y = (int) event->motion.y;

  got_motion = 1;

  return TRUE; /* TRUE means the event is handled. */
}

void cb_open_file(GtkWidget *w, gpointer data)
{
  GtkWidget *dialog;
  struct qp_qp *qp;
  struct qp_source *s = NULL;
  ASSERT(data);
  qp = data;

  dialog = gtk_file_chooser_dialog_new ("Open File",
		GTK_WINDOW(qp->window),
		GTK_FILE_CHOOSER_ACTION_OPEN,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
		NULL);
  if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
  {
    char *filename;
    filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
    ASSERT(filename && filename[0]);
    gtk_widget_destroy(dialog);

    s = qp_source_create(filename, QP_TYPE_UNKNOWN);

#if 0
#ifdef QP_DEBUG
    if(s)
      qp_source_debug_print(s);
#endif
#endif

    g_free(filename);
  }
  else
    gtk_widget_destroy(dialog);

  /* TODO: make defaults plots or open the plot builder gui */

  if(s && app->op_default_graph)
    qp_qp_graph_default_source(qp, s, NULL);
    

#if 0
  if(s)
    qp_qp_make_default_plots(qp, s);
#endif

}

void cb_remove_source(GtkWidget *w, gpointer data)
{
  ASSERT(data);
  qp_source_destroy((struct qp_source*)data);
}

void cb_new_window(GtkWidget *w, gpointer data)
{
  qp_qp_window(NULL, NULL);
}

void cb_copy_window(GtkWidget *w, gpointer data)
{
  ASSERT(data);
  qp_qp_copy((struct qp_qp*) data, qp_qp_window(NULL, NULL));
}

void cb_new_graph_tab(GtkWidget *w, gpointer data)
{
  ASSERT(data);
  qp_graph_create((struct qp_qp*)data, NULL);
}

void cb_save_png_image_file(GtkWidget *w, gpointer data)
{
  struct qp_qp *qp;
  struct qp_graph *gr;
  int page_num;
  char *filename = NULL;
  ASSERT(data);
  qp = data;
  page_num = gtk_notebook_get_current_page(GTK_NOTEBOOK(qp->notebook));
  w = gtk_notebook_get_nth_page(GTK_NOTEBOOK(qp->notebook), page_num);
  gr = g_object_get_data(G_OBJECT(w), "qp_graph");

  {
    GtkWidget *dialog;
    char name[64];
    snprintf(name, 64, "%.60s.png", gr->name);

    dialog = gtk_file_chooser_dialog_new ("Save PNG File",
				      GTK_WINDOW(qp->window),
				      GTK_FILE_CHOOSER_ACTION_SAVE,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
				      NULL);
    gtk_file_chooser_set_do_overwrite_confirmation(
        GTK_FILE_CHOOSER(dialog), TRUE);
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), name);
    if(gtk_dialog_run(GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
      filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
    gtk_widget_destroy (dialog);
  }

  if(filename)
  {
    qp_qp_save_png(qp, gr, filename);
    g_free(filename);
  }
}

