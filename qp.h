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


#ifndef PACKAGE_TARNAME
#  error  "You must include config.h before this file"
#endif

#include <stdint.h>
#include <sys/types.h>
#include <limits.h>
#include <X11/Xlib.h>
#include <gtk/gtk.h>
#include <cairo/cairo.h>


#define GRAB_BUTTON (1)
#define PICK_BUTTON (2)
#define ZOOM_BUTTON (3)


/* Given we just have one X server connection we just need
 * one saved pointer and mouse flag thingy. */
extern
int save_x, save_y,
    mouse_num, got_motion;



#ifdef INT
#error "INT is already defined"
#endif

/* ABS was defined already */
#define ABSVAL(x) (((x)>0)?(x):(-(x)))


#ifndef WINDOZ /* TODO: change to the windows defined thingy */
#define DIR_CHAR '/'
#define DIR_STR  "/"
#else
#define DIR_CHAR '\\'
#define DIR_STR  "\\"
#endif



/* converting the values to ints before plotting
 * increases draw rate by an order of magnitude!! */
#define INT(x)  ((int)(((x)>0.0)?(x)+0.5:(x)-0.5))


struct qp_zoom
{
  double xscale, xshift, yscale, yshift;
  struct qp_zoom *next;
};


/* color with alpha */
struct qp_colora
{
  double r, g, b, a;
};


struct qp_color
{
  struct qp_colora c; /* cairo colors */
  unsigned long x;    /* x11 color */
};



/* The plot class precomputes all the linear transformations
 * and reduces them to just one mulitple and one add for
 * each point value read in the tight plot loops where it counts. */

struct qp_plot
{
  struct qp_channel *x, *y;
  char *name;

  /* point and line colors */
  struct qp_color p, l;

  /* boolean to show lines and points */
  int lines, points;

  /* these are changed at the begining of each reading/plotting loop
   * They change with the current zoom that is passed in from the
   * graph */
  double xscale, yscale, xshift, yshift;
  /* these store the plot scales for the life of the plot
   * They never change after the plot is created */
  double xscale0, yscale0, xshift0, yshift0;

  /* This is used when we know how to limit the number
   * of values that we need to read from the channels
   * based on the nature of the data. */
  size_t num_read;


  double line_width, point_size;


  /* Virtual functions in C */

  /* This gives us a common interface to reading the
   * data whether the data be stored in floats, doubles
   * ints, and etc. */

  int (* x_is_reading)(struct qp_channel *c);
  int (* y_is_reading)(struct qp_channel *c);

  double (* channel_x_begin)(struct qp_channel *c);
  double (* channel_y_begin)(struct qp_channel *c);
  double (* channel_x_end)  (struct qp_channel *c);
  double (* channel_y_end)  (struct qp_channel *c);
  double (* channel_x_next) (struct qp_channel *c);
  double (* channel_y_next) (struct qp_channel *c);
  double (* channel_x_prev) (struct qp_channel *c);
  double (* channel_y_prev) (struct qp_channel *c);


  double (* channel_series_x_index)(struct qp_channel *c, size_t i);
  double (* channel_series_y_index)(struct qp_channel *c, size_t i);

  size_t (* channel_series_x_get_index)(struct qp_channel *c);

  
  //size_t (* num_vals)(struct qp_plot *p);
};


struct qp_source
{
  char *name;

  size_t num_values;
  int value_type; /* can be multi-type */

  size_t num_channels;

  /* An array of channels (pointers) */
  struct qp_channel **channels;
};



/* graphs do not exist unless they are
 * in a qp_qp with a gtk window. */
struct qp_graph
{
  char *name;

  struct qp_color_gen *color_gen;

  struct qp_sllist *plots;
  struct qp_qp *qp;

  GtkWidget *drawing_area,
            *tab_label,
            *tab_label_hbox,
            *close_button;

  /* xscale, xshift, yscale, yshift provide an
   * additional drawing area transformation that maps
   * to a sub area of the drawing area and flips Y
   * so it is positively changing in the up direction. */
  int xscale, xshift, yscale, yshift;


  /* The first zoom in the stack is the identity transformation.
   * Zooms are normalized to a 0,0 to 1,1 box */
  struct qp_zoom *z;

  /* bool value == do all the plots have the same scale? */
  int same_xscale, same_yscale;
  int show_grid, grid_numbers;
  /* maximum pixel space between grid lines
   * the smallest it could be is 1/3 of this */
  int grid_x_space, grid_y_space;
  int grid_line_width;
  int grid_on_top;


  char *grid_font;
  PangoLayout *pangolayout;


  int sig_fig_x, sig_fig_y;

  int zoom_level; /* starting at 0 */

  struct qp_colora background_color, grid_line_color, grid_text_color;

  /* A place to store the background color is we
   * switch to using X11 shape extension */
  double bg_alpha_preshape;

  int pixbuf_width, pixbuf_height,
      pixbuf_x, pixbuf_y;
  /* grab x,y needs to be a double so we may scale it on
   * window resize */
  double grab_x, grab_y;

  /* Defining relative positions of things:
   *
 ------------------------------------------------------------------------
 position            |position in  |position       |position
 in pixbuf           |drawing area |in scaled plots|in plot values
   (int)             |   (int)     |   (int)       |  (double)
 --------------------|-------------|---------------|----------------------
 (xi+pixbuf_x+grab_x,|(xi+grab_x,  |  (xi,yi)      |(qp_plot_get_xval(xi),
  yi+pixbuf_y+grab_y)| yi+grab_y)  |               | qp_plot_get_xval(xi))
 -------------------------------------------------------------------------

     scaling takes place in the qp_plot_get_*val() in the qp_plot objects
   */


  /* since we have our own drawing buffer which we double buffer with,
   * we do not need to redraw it at every draw event, hence this flag */
  int pixbuf_needs_draw;

  int draw_zoom_box;      // flag to tell to draw a zoom box
  int z_x, z_y, z_w, z_h; // zoom box geometry in drawing_area pixels

  int waiting_to_resize_draw;

  /* our back drawing buffer which may be larger than the drawing_area */
  cairo_surface_t *pixbuf_surface;
  cairo_t *cr; /* just tmp variable for drawing */
  void (*DrawLine)(struct qp_graph *gr, int *new_line,
      double from_x, double from_y, double to_x, double to_y);

  int font_antialias_set; /* bool */
  
  /* is NULL if not drawing with X11 */
  struct qp_graph_x11 *x11;
};

struct qp_graph_x11
{
  GC gc;
  Pixmap pixmap; /* sits under the pixbuf_surface */
  Display *dsp;

  /* background is used to store the background color
   * in raw format for the X11 shape extension draw
   * mode. */
  uint32_t background;
  int background_set;
};

/* there can be only one app object */
struct qp_app
{
  int *argc;
  char ***argv;

  int is_gtk_init;
  int main_window_count;

  struct qp_sllist *qps;
  struct qp_sllist *sources;


/* This include declares op_ variables which are long
 * options like for example:
 * --grid-line-width ==> op_grip_line_width
 *
 *  app_op_declare.h is a generated file. */
# include "app_op_declare.h"

  int op_grid_on_top;


  /* graph/plot defaults */
  

  double op_plot_line_width, op_plot_point_width; /* =NAN for auto */

  GdkCursor *waitCursor,
            *grabCursor,
            *holdCursor,
            *pickCursor,
            *zoomCursor;

 
  int root_window_width, root_window_height;

  int gui_can_exit;
};

/* There is one of these per quickplot main window
 * The window does not have to be made. */
struct qp_qp
{
  struct qp_sllist *graphs;
  struct qp_graph *current_graph;

  GtkWidget *window,
            *view_buttonbar,
            *view_menubar,
            *view_graph_tabs,
            *view_statusbar,
            *view_border,
            *view_fullscreen,
            *view_shape,
            *view_x11_draw,
            *view_cairo_draw,
            *delete_window_menu_item,
            *menubar,
            *file_menu,
            *buttonbar,
            *notebook, /* graph tabs */
            *statusbar,
            *status_entry;

  //int wait_warning_showing;

  /* this is the last known pointer position, in pixels,
   * in the drawing_area, all drawing areas being in the
   * same position and size for a given qp_qp. */
  int pointer_x, pointer_y;

  int wait_cursor_showing;

  /* boolean  draw graphs with X11 API */
  int x11_draw;

  int border;
  int shape; /* use X11 shape extention */

  /* This is just when we use the X shape extension.
   * We save the last shape that was applied and
   * do not apply a new shape region unless it
   * changes.  Otherwise we can end up with many
   * unnecessary redraw events. */
  cairo_region_t *last_shape_region;
};


/* this is the one and only gtk app
 * object. */
extern
struct qp_app *app;

extern
struct qp_app *qp_app_create(void);


extern
void qp_getargs_1st_pass(int argc, char **argv);

extern
void qp_getargs_2nd_pass(int argc, char **argv);


extern
void qp_qp_copy(struct qp_qp *old_qp, struct qp_qp *new_qp);

extern
int qp_find_doc_file(const char *fileName, char **fullpath_ret);

extern
int qp_launch_browser(const char *fileName);

extern
void qp_get_root_window_size(void);

extern
void qp_qp_set_status(struct qp_qp *qp);

extern
void add_source_buffer_remove_menus(struct qp_source *source);


#ifdef QP_DEBUG
extern
void qp_source_debug_print(struct qp_source *s);
#endif

extern
void qp_plot_destroy(struct qp_plot *plot, struct qp_graph *gr);


extern
int qp_source_parse_doubles(struct qp_source *source, char *line_in);

extern
void qp_graph_zoom_out(struct qp_graph *gr, int all);

extern
void qp_graph_switch_draw_mode(struct qp_graph *gr);


extern
void qp_qp_graph_remove(qp_qp_t qp, qp_graph_t graph);

/* plots belong to the creating graph */
extern
qp_plot_t qp_plot_create(qp_graph_t graph,
    qp_channel_t x, qp_channel_t y, const char *name,
    double xmin, double xmax, double ymin, double ymax);

extern
qp_graph_t qp_graph_create(qp_qp_t qp, const char *name);

extern
void qp_graph_grid_draw(struct qp_graph *gr,
    struct qp_plot *p,
    cairo_t *cr, int width, int height);

extern
void qp_graph_destroy(qp_graph_t graph);

extern
void qp_graph_append(qp_graph_t graph, qp_plot_t  plot);


extern
void qp_graph_draw(struct qp_graph *gr, cairo_t *cr);


extern
int qp_qp_save_png(struct qp_qp *qp,
    struct qp_graph *gr, const char *filename);

/** 
 * Sources can be shared between qps
 *
 * value_type will be ignored if the value_type can
 * be easily determined from the file contents. */
extern
qp_source_t qp_source_create(
    const char *filename, int value_type);

extern
qp_source_t qp_source_create_from_func(
    const char *name, int value_type,
    void (* func)(const void *));
  
extern
void qp_source_destroy(qp_source_t source);


extern
struct qp_gtk_options *strip_gtk_options(int *argc, char ***argv);

extern
int qp_gtk_init_check(struct qp_gtk_options *opt);


/* default_qp is the last qp created */
extern
struct qp_qp *default_qp;

static inline
struct qp_qp *qp_qp_check(struct qp_qp *qp)
{
  if(qp)
    return qp;
  if(default_qp)
    return default_qp;
  return default_qp = qp_qp_create();
}

static inline
void qp_app_check(void)
{
  if(!app)
    qp_app_create();
}

