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

#include "channel.h"
#include "channel_double.h"
#include "plot.h"


static __thread int _cb_view_graph_detail_reenter = 0;

static
gboolean graph_detail_hide(struct qp_qp *qp)
{
  ASSERT(qp);
  ASSERT(qp->graph_detail);
  ASSERT(qp->graph_detail->window);

  if(qp->graph_detail->window)
    gtk_widget_hide(qp->graph_detail->window);
  return TRUE; /* TRUE means event is handled */
}

static inline
void set_color_button(GtkWidget *w, struct qp_colora *c)
{
  GdkRGBA rgba;

  rgba.red = c->r;
  rgba.green = c->g;
  rgba.blue = c->b;
  rgba.alpha = c->a;
  //DEBUG("%g %g %g %g\n", c->r, c->g, c->b, c->a);
  gtk_color_button_set_rgba(GTK_COLOR_BUTTON(w), &rgba);
}

static inline
void set_combo_box_text(GtkWidget *w, int same_scale,
    int same_limits)
{
  ASSERT((same_limits && same_scale) || !same_limits);


  gtk_combo_box_set_button_sensitivity(GTK_COMBO_BOX(w), GTK_SENSITIVITY_ON);
  gtk_combo_box_set_active(GTK_COMBO_BOX(w), same_scale);

  if(same_limits)
    gtk_combo_box_set_button_sensitivity(GTK_COMBO_BOX(w), GTK_SENSITIVITY_OFF);
  else
    gtk_combo_box_set_button_sensitivity(GTK_COMBO_BOX(w), GTK_SENSITIVITY_ON);
}

static
void set_slider_val(int val, struct qp_slider *s)
{
  if(s->val)
    *(s->val) = val;
  else if(s->is_plot_line_width)
  {
    struct qp_sllist *plots;
    struct qp_plot *p;
    ASSERT(s->gr);
    plots = s->gr->plots;
    for(p=qp_sllist_begin(plots);p;p=qp_sllist_next(plots))
      p->line_width = val;
  }
  else
  {
    struct qp_sllist *plots;
    struct qp_plot *p;
    ASSERT(s->gr);
    plots = s->gr->plots;
    for(p=qp_sllist_begin(plots);p;p=qp_sllist_next(plots))
      p->point_size = val;
  }
}

static __thread int _ignore_slider_cb = 0;

static
void cb_text_entry(GtkEntry *entry, struct qp_slider *s)
{
  char str[8], *ss;
  const char *text;
  int val;
  double dval;

  if(_ignore_slider_cb)
    return;

  text = gtk_entry_get_text(entry);

  strncpy(str, text, 7);
  str[7] = '\0';

  for(ss = str; *ss; ++ss)
    if(*ss < '0' && *ss > '9')
    {
      /* Remove non-number chars */
      char *c;
      for(c = ss; *c; ++c)
        *c = *(c+1);
    }


  val = strtol(str, NULL, 10);

  if(val < s->min)
    val = s->min;
  else if(val > s->max)
  {
    if(val > s->extended_max)
      val = s->max = s->extended_max;
    else
      s->max = val;
  }

  snprintf(str, 8, "%d", val);

  if(strcmp(str, text))
    gtk_entry_set_text(entry, str);

  dval = s->max - s->min;
  dval = val/dval - s->min/dval;

  _ignore_slider_cb = 1;
  gtk_range_set_value(GTK_RANGE(s->scale), dval);
  _ignore_slider_cb = 0;

  set_slider_val(val, s);
}

static
void set_slider_entry(struct qp_slider *slider, int val)
{
  char text[8];
  snprintf(text, 8, "%d", val);
  gtk_entry_set_text(GTK_ENTRY(slider->entry), text);
  cb_text_entry(GTK_ENTRY(slider->entry), slider);
}

/* Setup the widget for a particular graph */
void qp_qp_graph_detail_init(struct qp_qp *qp)
{
  struct qp_graph *gr;
  struct qp_graph_detail *gd;

  ASSERT(qp);
  ASSERT(qp->graph_detail);
  ASSERT(qp->graph_detail->window);

  gd = qp->graph_detail;
  gr = qp->current_graph;

  {
    char title[256];
    snprintf(title, 256, "%s Graph Details", gr->name);
    gtk_window_set_title(GTK_WINDOW(gd->window), title);
  }
  {
    char text[128];        
    snprintf(text, 128, "Configure Graph: %s", gr->name);
    gtk_label_set_text(GTK_LABEL(gd->config_label), text);
  }

  {
    /**************************************************
     *       Setup Show Check Buttons
     **************************************************/
    struct qp_plot *p;
    int lines = 0, points = 0;
    GList *list, *l;
    GtkWidget *w;
    
    for(p=qp_sllist_begin(gr->plots);p;p=qp_sllist_next(gr->plots))
    {
      if(p->lines)
        lines = 1;
      if(p->points)
        points = 1;
    }

    /* Set to NULL so that the callbacks do not do anything. */
    qp->current_graph = NULL;

    l = list = gtk_container_get_children(GTK_CONTAINER(gd->show_container));
    /* Show Grid */
    w = l->data;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), gr->show_grid);
    /* Show Grid Numbers */
    l = l->next;
    w = l->data;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), gr->grid_numbers);
    /* Show Lines */
    l = l->next;
    w = l->data;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), lines);
    /* Show Points */
    l = l->next;
    w = l->data;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), points);
    /************************/
    g_list_free(list);
  }

  set_color_button(gd->background_color_picker,&(gr->background_color));
  set_color_button(gd->grid_color_picker,&(gr->grid_line_color));
  set_color_button(gd->numbers_color_picker,&(gr->grid_text_color));
  gtk_font_button_set_font_name(GTK_FONT_BUTTON(gd->font_picker), gr->grid_font);

  set_combo_box_text(gd->x_scale, gr->same_x_scale, gr->same_x_limits);
  set_combo_box_text(gd->y_scale, gr->same_y_scale, gr->same_y_limits);


  gd->line_width_slider->gr = gr;
  gd->line_width_slider->is_plot_line_width = 1;
  if(qp_sllist_first(gr->plots))
    set_slider_entry(gd->line_width_slider,
      ((struct qp_plot*)qp_sllist_first(gr->plots))->line_width);
  else
    set_slider_entry(gd->line_width_slider, 4);
  
  gd->point_size_slider->gr = gr;
  if(qp_sllist_first(gr->plots))
    set_slider_entry(gd->point_size_slider,
      ((struct qp_plot*)qp_sllist_first(gr->plots))->point_size);
  else
    set_slider_entry(gd->point_size_slider, 4);

  gd->grid_line_width_slider->val = &(gr->grid_line_width);
  set_slider_entry(gd->grid_line_width_slider, gr->grid_line_width);
  gd->grid_x_line_space_slider->val = &(gr->grid_x_space);
  set_slider_entry(gd->grid_x_line_space_slider, gr->grid_x_space);
  gd->grid_y_line_space_slider->val = &(gr->grid_y_space);
  set_slider_entry(gd->grid_y_line_space_slider, gr->grid_y_space);

  gtk_widget_queue_draw(qp->graph_detail->selector_drawing_area);

  qp->current_graph = gr;
}

static
void cb_show_grid(GtkWidget *w, void *data)
{
  struct qp_graph *gr;
  gr = ((struct qp_qp *)data)->current_graph;

  if(!gr)
    return;

  gr->show_grid = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));
  DEBUG("on=%d\n", gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)));
}

static
void cb_show_grid_numbers(GtkWidget *w, void *data)
{
  struct qp_graph *gr;
  gr = ((struct qp_qp *)data)->current_graph;

  if(!gr)
    return;

 gr->grid_numbers = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));
  DEBUG("on=%d\n", gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)));
}

static
void cb_show_lines(GtkWidget *w, void *data)
{
  struct qp_graph *gr;
  struct qp_plot *p;
  int on;
  gr = ((struct qp_qp *)data)->current_graph;

  if(!gr)
    return;

  on = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));
  for(p=qp_sllist_begin(gr->plots);p;p=qp_sllist_next(gr->plots))
    p->lines = on;

  DEBUG("on=%d\n", on);
}

static
void cb_show_points(GtkWidget *w, void *data)
{
  struct qp_graph *gr;
  struct qp_plot *p;
  int on;
  gr = ((struct qp_qp *)data)->current_graph;

  if(!gr)
    return;

  on = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));
  for(p=qp_sllist_begin(gr->plots);p;p=qp_sllist_next(gr->plots))
    p->points = on;

  DEBUG("on=%d\n", on);
}

static inline
void set_rgba(GtkColorButton *w, struct qp_colora *c)
{
  GdkRGBA rgba;
  gtk_color_button_get_rgba(w, &rgba);
  c->r = rgba.red;
  c->g = rgba.green;
  c->b = rgba.blue;
  c->a = rgba.alpha;
}

static
void cb_background_color(GtkColorButton *w, gpointer data)
{
  struct qp_graph *gr;
  gr = ((struct qp_qp *)data)->current_graph;
  if(!gr)
    return;
  set_rgba(w, &(gr->background_color));
}

static
void cb_grid_color(GtkColorButton *w, gpointer data)
{
  struct qp_graph *gr;
  gr = ((struct qp_qp *)data)->current_graph;
  if(!gr)
    return;
  set_rgba(w, &(gr->grid_line_color));
}

static
void cb_grid_numbers_color(GtkColorButton *w, gpointer data)
{
  struct qp_graph *gr;
  gr = ((struct qp_qp *)data)->current_graph;
  if(!gr)
    return;
  set_rgba(w, &(gr->grid_text_color));
}

static
void cb_grid_font(GtkFontButton *w, gpointer data)
{
  struct qp_graph *gr;
  gr = ((struct qp_qp *)data)->current_graph;
  if(!gr)
    return;

  if(gr->grid_font)
    free(gr->grid_font);

  gr->grid_font = qp_strdup(gtk_font_button_get_font_name(w));

  if(gr->pangolayout)
    /* If there is no pangolayout there is no reason to call this */
    qp_graph_set_grid_font(gr);
}

static
void create_show_check_button(const char *label,
    void (*callback)(GtkWidget*, void*), struct qp_qp *qp,
    GtkWidget *vbox, gboolean checked)
{
  GtkWidget *b;
  b = gtk_check_button_new_with_label(label);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(b), checked);

  if(callback)
    g_signal_connect(G_OBJECT(b), "toggled", G_CALLBACK(callback), qp);
  gtk_box_pack_start(GTK_BOX(vbox), b, FALSE, FALSE, 0);
  gtk_widget_show(b);
}

static
gboolean ecb_close_graph_detail(
    GtkWidget *w, GdkEvent *event, gpointer data)
{
  cb_graph_detail_show_hide(NULL, data);
  DEBUG("\n");
  return TRUE;/* TRUE means event is handled */
}

static
void cb_same_x_scale(GtkComboBox *w, gpointer data)
{
  struct qp_graph *gr;
  struct qp_plot *p;
  double min=INFINITY, max=-INFINITY;

  gr = ((struct qp_qp *)data)->current_graph;
  if(!gr)
    return;
  
  ASSERT(!gr->same_x_limits);

  gr->same_x_scale = gtk_combo_box_get_active(w);

  if(gr->same_x_scale)
  {
    for(p=qp_sllist_begin(gr->plots);p;p=qp_sllist_next(gr->plots))
    {
      ASSERT(p->x->form == QP_CHANNEL_FORM_SERIES);
      if(max > p->x->series.max)
        max = p->x->series.max;
      if(min < p->x->series.min)
        min = p->x->series.min;
    }
    if(max == min)
    {
      max += 1;
      min -= 1;
    }
    else if(max - min < SMALL_DOUBLE)
    {
      max += SMALL_DOUBLE;
      min -= SMALL_DOUBLE;
    }
  }

  for(p=qp_sllist_begin(gr->plots);p;p=qp_sllist_next(gr->plots))
      qp_plot_x_rescale(p, min, max);
}

static
void cb_same_y_scale(GtkComboBox *w, gpointer data)
{
  struct qp_graph *gr;

  gr = ((struct qp_qp *)data)->current_graph;
  if(!gr)
    return;
  
  ASSERT(!gr->same_y_limits);

  gr->same_y_scale = gtk_combo_box_get_active(w);

  if(gr->same_y_scale)
  {
    struct qp_plot *p;
    double min=INFINITY, max=-INFINITY;

    for(p=qp_sllist_begin(gr->plots);p;p=qp_sllist_next(gr->plots))
    {
      ASSERT(p->y->form == QP_CHANNEL_FORM_SERIES);
      if(max < p->y->series.max)
        max = p->y->series.max;
      if(min > p->y->series.min)
        min = p->y->series.min;
    }
    if(max == min)
    {
      max += 1;
      min -= 1;
    }
    else if(max - min < SMALL_DOUBLE)
    {
      max += SMALL_DOUBLE;
      min -= SMALL_DOUBLE;
    }
    for(p=qp_sllist_begin(gr->plots);p;p=qp_sllist_next(gr->plots))
      qp_plot_y_rescale(p, min, max);

  }
  else
  {
    struct qp_plot *p;
    for(p=qp_sllist_begin(gr->plots);p;p=qp_sllist_next(gr->plots))
    {
      double min, max;
      max = p->y->series.max;
      min = p->y->series.min;
      if(max == min)
      {
        max += 1;
        min -= 1;
      }
      else if(max - min < SMALL_DOUBLE)
      {
        max += SMALL_DOUBLE;
        min -= SMALL_DOUBLE;
      }

      qp_plot_y_rescale(p, min, max);
    }
  }

}


static
void cb_scale_change(GtkRange *range, struct qp_slider *s)
{
  int val;
  char str[8];

  if(_ignore_slider_cb)
    return;

  val = INT((s->max - s->min)*gtk_range_get_value(range) + s->min);
  snprintf(str, 8, "%d", val);

  _ignore_slider_cb = 1;
  gtk_entry_set_text(GTK_ENTRY(s->entry), str);
  _ignore_slider_cb = 0;

  set_slider_val(val, s);
}

static
struct qp_slider *create_slider_input(const char *label,
    GtkWidget *vbox, int min, int max, int extended_max)
{
  GtkWidget *frame, *scale, *entry, *hbox;
  int max_len;
  double v,step;
  char text[16];
  struct qp_slider *slider;

  slider = qp_malloc(sizeof(*slider));
  slider->min = min;
  slider->max = max;
  slider->extended_max = extended_max;
  slider->val = NULL;
  slider->is_plot_line_width = 0;
  slider->gr = NULL;

  frame = gtk_frame_new(label);
  hbox = gtk_hbox_new(FALSE, 2);

  v = 0.3;
  step = 1.0/((double)(max - min));

  scale = gtk_hscale_new_with_range(0.0, 1.0, 0.001);
  gtk_range_set_increments(GTK_RANGE(scale), step, 2*step);
  gtk_range_set_value(GTK_RANGE(scale), v);
  gtk_widget_set_size_request(scale, 100, -1);
  gtk_scale_set_draw_value(GTK_SCALE(scale), FALSE);
  g_signal_connect(G_OBJECT(scale), "value-changed", G_CALLBACK(cb_scale_change), slider);
  gtk_box_pack_start(GTK_BOX(hbox), scale, TRUE, TRUE, 2);
  gtk_widget_show(scale);

  entry = gtk_entry_new();
  snprintf(text, 16, "%d", extended_max);
  max_len = strlen(text);
  ASSERT(max_len <= 4);
  gtk_entry_set_max_length(GTK_ENTRY(entry), max_len);
  gtk_entry_set_width_chars(GTK_ENTRY(entry), 4);
  //g_signal_connect(G_OBJECT(entry), "focus-out-event", G_CALLBACK(cb_text_entry_focus_out), slider);
  g_signal_connect(G_OBJECT(entry), "activate", G_CALLBACK(cb_text_entry), slider);
  gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 2);
  gtk_widget_show(entry);

  slider->scale = scale;
  slider->entry = entry;

  gtk_container_add(GTK_CONTAINER(frame), hbox);
  gtk_widget_show(hbox);
  gtk_widget_show(frame);
  gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 8);
  return slider;
}

static
void cb_redraw(GtkButton *button, struct qp_qp *qp)
{
  gtk_widget_queue_draw(qp->current_graph->drawing_area);
  qp->current_graph->pixbuf_needs_draw = 1;
  gdk_window_set_cursor(gtk_widget_get_window(qp->window), app->waitCursor);
}

static inline
void add_grid_label(GtkWidget *grid, const char *text,
    int with_frame, int vertical, int left, int top)
{
  GtkWidget *label, *frame;
  label = gtk_label_new(text);

  if(vertical)
    gtk_label_set_angle(GTK_LABEL(label), 80);

  frame = gtk_frame_new(NULL);


  if(!with_frame)
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_NONE);
  
  gtk_container_add(GTK_CONTAINER(frame), label);
  gtk_widget_show(label);

  gtk_grid_attach(GTK_GRID(grid), frame, left, top, 1, 1);

  gtk_widget_show(frame);
}


struct qp_plotter
{
  struct qp_qp *qp;
  GtkWidget *radio;
  ssize_t channel_num;
  struct qp_source *source;
  struct qp_channel *channel;
  int is_y; /* 0 = x  1 = y */
};

static
void cb_plotter(GtkButton *w, struct qp_plotter *pr)
{
  GtkWidget *vbox;
  GList *l, *list;
  struct qp_plot *p;
  struct qp_graph *gr;
  struct qp_plotter *o_pr = NULL, *x_pr = NULL, *y_pr = NULL;

  if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)))
    return;

  /* Get the other channel */
  if(pr->is_y)
  {
    vbox = pr->qp->graph_detail->selector_x_vbox;
    y_pr = pr;
  }
  else
  {
    vbox = pr->qp->graph_detail->selector_y_vbox;
    x_pr = pr;
  }
  l = list = gtk_container_get_children(GTK_CONTAINER(vbox));
  /* first check the "none" radio button */
  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(l->data)))
    return; /* It is "none" */

  for(l=list->next;l;l=l->next)
  {
    o_pr = g_object_get_data(G_OBJECT(l->data), "plotter");
    if(o_pr && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(l->data)))
      break;
    else
      o_pr = NULL;
  }
  g_list_free(list);

  ASSERT(o_pr);
  if(!o_pr)
    return; /* WTF */

  if(x_pr)
    y_pr = o_pr;
  else
    x_pr = o_pr;

  gr = pr->qp->current_graph;

  /* we have the two channels x_pr and y_pr */
  for(p=qp_sllist_begin(gr->plots);p;p=qp_sllist_next(gr->plots))
    if(qp_channel_equals(p->x, x_pr->channel) && 
        qp_channel_equals(p->y, y_pr->channel))
      break;

  if(p)
  {
    qp_sllist_remove(gr->plots, p, 0);
    qp_plot_destroy(p, gr);
  }
  else
  {
    char pname[128];
    ASSERT(x_pr->channel->form == QP_CHANNEL_FORM_SERIES);
    ASSERT(y_pr->channel->form == QP_CHANNEL_FORM_SERIES);
    snprintf(pname, 128, "%s-%zu VS %s-%zu",
        x_pr->source->name, x_pr->channel_num,
        y_pr->source->name, y_pr->channel_num);
    qp_plot_create(gr, x_pr->channel, y_pr->channel , pname,
        x_pr->channel->series.min, x_pr->channel->series.max,
        y_pr->channel->series.min, y_pr->channel->series.max);
  }


  if(qp_sllist_length(gr->plots))
  {
    /* We must rescale all the plots */
    /* TODO: make this code suck less */
    double dx_min = INFINITY, xmin = INFINITY, xmax = -INFINITY,
           dy_min = INFINITY, ymin = INFINITY, ymax = -INFINITY;
    struct qp_channel_series *csx0, *csy0;
    p=qp_sllist_begin(gr->plots);
    ASSERT(p->x->form == QP_CHANNEL_FORM_SERIES);
    ASSERT(p->y->form == QP_CHANNEL_FORM_SERIES);
    csx0 = &(p->x->series);
    csy0 = &(p->y->series);
    gr->same_x_limits = 1;
    gr->same_y_limits = 1;

    for(p=qp_sllist_begin(gr->plots);p;p=qp_sllist_next(gr->plots))
    {
      double dx, dy;
      struct qp_channel_series *cs;
      ASSERT(p->x->form == QP_CHANNEL_FORM_SERIES);
      ASSERT(p->y->form == QP_CHANNEL_FORM_SERIES);
      cs = &(p->x->series);
      dx = cs->max - cs->min;
      if(xmin > cs->min)
        xmin = cs->min;
      if(xmax < cs->max)
        xmax = cs->max;
      if(dx > SMALL_DOUBLE && dx_min > dx)
        dx_min = dx;
      if(csx0->max != cs->max || csx0->min != cs->min)
        gr->same_x_limits = 0;

      cs = &(p->y->series);
      dy = cs->max - cs->min;
      if(ymin > cs->min)
        ymin = cs->min;
      if(ymax < cs->max)
        ymax = cs->max;
      if(dy > SMALL_DOUBLE && dy_min > dy)
        dy_min = dy;
      if(csy0->max != cs->max || csy0->min != cs->min)
        gr->same_y_limits = 0;
    }

    if(gr->same_x_limits)
    {
      gr->same_x_scale = 1;
    }

    if(gr->same_x_scale)
    {
      if(xmax == xmin)
      {
        /* It could be just one point so lets give it a
         * decent scale, not like the 1e-14 crap. */
        xmax += 1;
        xmin -= 1;
      }
      else if(xmax - xmin < SMALL_DOUBLE)
      {
        xmax += SMALL_DOUBLE;
        xmin -= SMALL_DOUBLE;
      }
    }
    else if(xmax == xmin)
    {
      /* It could be just one point so lets give it a
       * decent scale, not like the 1e-14 crap. */
      xmax += 1;
      xmin -= 1;
    }
    else if(xmax - xmin < SMALL_DOUBLE)
    {
      /* they requested different scales but the values
       * are too close together, so we make it same scale */
      xmax += SMALL_DOUBLE;
      xmin -= SMALL_DOUBLE;
    }
    else
    {
      /* use different scales */
      xmin = INFINITY;
      xmax = -INFINITY;
    }


    if(gr->same_y_limits)
    {
      gr->same_y_scale = 1;
    }

    if(gr->same_y_scale)
    {
      if(ymax == ymin)
      {
        /* It could be just one point so lets give it a
         * decent scale, not like the 1e-14 crap. */
        ymax += 1;
        ymin -= 1;
      }
      else if(ymax - ymin < SMALL_DOUBLE)
      {
        ymax += SMALL_DOUBLE;
        ymin -= SMALL_DOUBLE;
      }
    }
    else if(ymax == ymin)
    {
      /* It could be just one point so lets give it a
       * decent scale, not like the 1e-14 crap. */
      ymax += 1;
      ymin -= 1;
    }
    else if(ymax - ymin < SMALL_DOUBLE)
    {
      /* they requested different scales but the values
       * are too close together, so we make it same scale */
      ymax += SMALL_DOUBLE;
      ymin -= SMALL_DOUBLE;
    }
    else
    {
      /* use different scales */
      ymin = INFINITY;
      ymax = -INFINITY;
    }

    for(p=qp_sllist_begin(gr->plots);p;p=qp_sllist_next(gr->plots))
    {
      qp_plot_x_rescale(p, xmin, xmax);
      qp_plot_y_rescale(p, ymin, ymax);
    }
  }

  gtk_widget_queue_draw(gr->drawing_area);
  gr->pixbuf_needs_draw = 1;
  gdk_window_set_cursor(gtk_widget_get_window(gr->qp->window), app->waitCursor);

  gtk_widget_queue_draw(gr->qp->graph_detail->selector_drawing_area);

  //qp_qp_graph_detail_init(gr->qp);

  /* Disable the graph checking when we reset the combo_box widget */
  gr->qp->current_graph = NULL;
  set_combo_box_text(gr->qp->graph_detail->x_scale, gr->same_x_scale, gr->same_x_limits);
  set_combo_box_text(gr->qp->graph_detail->y_scale, gr->same_y_scale, gr->same_y_limits);
  gr->qp->current_graph = gr;

#if 0
  DEBUG("chan_num=%zd toggled=%d\n",
      pr->channel_num,
      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)));
#endif
}

static
GtkWidget *make_channel_selector_column(GtkWidget *hbox, struct qp_qp *qp, int is_y)
{
  GtkWidget *radio, *vbox;
  struct qp_source *s;

  vbox = gtk_vbox_new(FALSE, 0);

  radio = gtk_radio_button_new_with_label(NULL, "none");
  gtk_box_pack_start(GTK_BOX(vbox), radio, FALSE, FALSE, 0);
  gtk_widget_show(radio);

  for(s=qp_sllist_begin(app->sources);s;s=qp_sllist_next(app->sources))
  {
    size_t i;
    GtkWidget *label;

    label = gtk_label_new(s->name);
    gtk_widget_set_margin_top(label, 8);
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 8);
    gtk_widget_show(label);

    for(i=0;i<s->num_channels;++i)
    {
      char text[128];
      struct qp_plotter *pr;
      snprintf(text, 128, "%s [%zu]", s->name, i);
      radio = gtk_radio_button_new_with_label_from_widget(
            GTK_RADIO_BUTTON(radio), text);
      pr = qp_malloc(sizeof(*pr));
      pr->qp = qp;
      pr->source = s;
      pr->radio = radio;
      pr->channel = s->channels[i];
      pr->channel_num = i;
      pr->is_y = is_y;
      g_object_set_data(G_OBJECT(radio), "plotter", pr);
      g_signal_connect(G_OBJECT(radio), "clicked", G_CALLBACK(cb_plotter), pr);
      gtk_widget_set_margin_top(radio, 0);
      gtk_widget_set_margin_bottom(radio, 0);
      gtk_box_pack_start(GTK_BOX(vbox), radio, FALSE, FALSE, 0);
      gtk_widget_show(radio);
    }
  }

  gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);
  gtk_widget_show(vbox);
  return vbox;
}

static
gboolean cb_selector_draw(GtkWidget *w, cairo_t *cr, struct qp_qp *qp)
{
  struct qp_graph *gr;
  GtkAllocation da_a;
  GList *l, *list;
  struct qp_plot *p;

  gtk_widget_get_allocation(w, &da_a);

  gr = qp->current_graph;

  cairo_set_source_rgba(cr, gr->background_color.r,
      gr->background_color.g, gr->background_color.b,
      gr->background_color.a);
  cairo_paint(cr);

  cairo_set_source_rgba(cr, gr->grid_line_color.r,
      gr->grid_line_color.g, gr->grid_line_color.b,
      gr->grid_line_color.a);

  l = gtk_container_get_children(
      GTK_CONTAINER(qp->graph_detail->selector_x_vbox));
  /* The first toggle button is the "none" button. */
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(l->data), TRUE);
  g_list_free(l);


  l = list = gtk_container_get_children(
      GTK_CONTAINER(qp->graph_detail->selector_y_vbox));
  /* The first toggle button is the "none" button. */
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(l->data), TRUE);


  /* Set the X and Y Channel "none" radio buttons. */
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(l->data), TRUE);

  cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);

  /* skip the "none" radio button */
  for(l=list->next;l;l=l->next)
  {
    GtkAllocation a;
    gtk_widget_get_allocation(GTK_WIDGET(l->data), &a);
    if(g_object_get_data(G_OBJECT(l->data), "plotter"))
    {
      cairo_rectangle(cr, 0, a.y - da_a.y + 2, a.height/2, a.height-4);
      cairo_rectangle(cr, da_a.width - a.height/2, a.y - da_a.y + 2,
            a.height/2, a.height-4);
    }
    else
      cairo_rectangle(cr, 0, a.y - da_a.y, da_a.width, a.height);
  }
  cairo_fill(cr);

  for(p=qp_sllist_begin(gr->plots);p;p=qp_sllist_next(gr->plots))
  {
    int x_y = 0, y_y = 0, width;
    double w2;
   
    for(l=list->next;l;l=l->next)
    {
      struct qp_plotter *pr;
      pr = g_object_get_data(G_OBJECT(l->data), "plotter");
      if(!x_y && pr && qp_channel_equals(pr->channel, p->x))
      {
        GtkAllocation a;
        gtk_widget_get_allocation(GTK_WIDGET(l->data), &a);
        x_y = a.y - da_a.y + a.height/2;
        if(y_y)
          break;
      }
      if(!y_y && pr && qp_channel_equals(pr->channel, p->y))
      {
        GtkAllocation a;
        gtk_widget_get_allocation(GTK_WIDGET(l->data), &a);
        y_y = a.y - da_a.y + a.height/2;
        if(x_y)
          break;
      }
    }
    width = p->line_width;
    if(width > 22)
      width = 22;

    cairo_set_source_rgba(cr, p->l.c.r, p->l.c.g, p->l.c.b, p->l.c.a);
    cairo_set_line_width(cr, width);
    cairo_move_to(cr, 0, x_y);
    cairo_line_to(cr, da_a.width, y_y);
    cairo_stroke(cr);

    width = p->point_size;
    if(width > 22)
      width = 22;

    w2 = width/2.0;

    cairo_set_source_rgba(cr, p->p.c.r, p->p.c.g, p->p.c.b, p->p.c.a);
    cairo_rectangle(cr, da_a.width/3 - w2, x_y + (y_y - x_y)/3 - w2,
        width, width);
    cairo_rectangle(cr, 2*da_a.width/3 - w2, x_y + 2*(y_y - x_y)/3 - w2,
        width, width);
    cairo_fill(cr);
  }

  g_list_free(list);

  return TRUE; /* TRUE means the event is handled. */
}


static
void plot_selector_make(struct qp_qp *qp)
{
  GtkWidget *hbox, *da;
  const GdkRGBA rgba = { 0.0, 0.0, 0.0, 1.0 };

  hbox = qp->graph_detail->selector_hbox;

  qp->graph_detail->selector_x_vbox = make_channel_selector_column(hbox, qp, 0);

  qp->graph_detail->selector_drawing_area = da = gtk_drawing_area_new();
  g_signal_connect(G_OBJECT(da), "draw", G_CALLBACK(cb_selector_draw), qp);
  gtk_box_pack_start(GTK_BOX(hbox), da, TRUE, TRUE, 0);
  gtk_widget_override_background_color(da, GTK_STATE_FLAG_NORMAL, &rgba);
  gtk_widget_show(da);

  qp->graph_detail->selector_y_vbox = make_channel_selector_column(hbox, qp, 1);
}

/* Call this if there are sources added or removed
 * and qp->graph_detail is non NULL. */
static
void qp_plot_selector_remake(struct qp_qp *qp)
{
  GList *l, *list;

  l = list = gtk_container_get_children(
      GTK_CONTAINER(qp->graph_detail->selector_x_vbox));
  for(l=list->next;l;l=l->next)
  {
    void *pr;
    pr = g_object_get_data(G_OBJECT(l->data), "plotter");
    if(pr)
      free(pr);
  }
  g_list_free(list);

  l = list = gtk_container_get_children(
      GTK_CONTAINER(qp->graph_detail->selector_y_vbox));

  for(l=list->next;l;l=l->next)
  {
    void *pr;
    pr = g_object_get_data(G_OBJECT(l->data), "plotter");
    if(pr)
      free(pr);
  }
  g_list_free(list);

  /* Remove the widgets from the selector_hbox */
  list = l = gtk_container_get_children(
        GTK_CONTAINER(qp->graph_detail->selector_hbox));
  for(;l && l->data;l=l->next)
    gtk_widget_destroy(GTK_WIDGET(l->data));
  g_list_free(list);


  plot_selector_make(qp);
}

void qp_app_plot_selectors_remake(void)
{
  struct qp_qp *qp;
  ASSERT(app);
  ASSERT(app->qps);

  for(qp=qp_sllist_begin(app->qps);qp;qp=qp_sllist_next(app->qps))
    if(qp->graph_detail)
      qp_plot_selector_remake(qp);
}

static
GtkWidget *make_pretty_header_label(const char *text, GtkBox *vbox)
{
  GtkWidget *hbox, *frame, *label;

  hbox = gtk_hbox_new(TRUE, 14);
  frame = gtk_frame_new(NULL);
  label = gtk_label_new(text);

  gtk_container_add(GTK_CONTAINER(frame), label);
  gtk_widget_show(label);
  gtk_box_pack_start(GTK_BOX(hbox), frame, TRUE, TRUE, 8);
  gtk_widget_show(frame);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 10);
  gtk_widget_show(hbox);
  return label;
}

static
void make_redraw_button(GtkWidget *vbox, struct qp_qp *qp)
{
  GtkWidget *button, *hbox;
  hbox = gtk_hbox_new(TRUE, 14);
  button = gtk_button_new_with_label("Redraw Graph");
  g_signal_connect(button, "clicked", G_CALLBACK(cb_redraw), qp);
  gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 8);
  gtk_widget_show(button);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 8);
  gtk_widget_show(hbox);
}

/* Make a graph detail main window widget */
static
void graph_detail_create(struct qp_qp *qp)
{
  struct qp_graph *gr;
  struct qp_graph_detail *gd;

  ASSERT(qp);

  gr = qp->current_graph;

  ASSERT(!qp->graph_detail);

  gd = (qp->graph_detail = qp_malloc(sizeof(*gd)));

  gd->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  gtk_window_set_icon(GTK_WINDOW(gd->window),
      gtk_window_get_icon(GTK_WINDOW(qp->window)));

  gtk_window_set_default_size(GTK_WINDOW(gd->window), 600, -1);

  g_signal_connect(G_OBJECT(gd->window), "delete_event",
      G_CALLBACK(ecb_close_graph_detail), qp);
  g_signal_connect(G_OBJECT(gd->window), "key-press-event",
      G_CALLBACK(ecb_key_press), qp);

  {
    GtkWidget *notebook;

    notebook =  gtk_notebook_new();
    gtk_notebook_set_show_border(GTK_NOTEBOOK(qp->notebook), FALSE);
    g_object_set(G_OBJECT(notebook), "scrollable", TRUE, NULL);

    {
      /*************************************************************************
       *                   Configure Graph
       *************************************************************************/

      GtkWidget *vbox;
      GtkWidget *tab_label;
      GtkWidget *hbox;

      vbox = gtk_vbox_new(FALSE, 0);

      gd->config_label = make_pretty_header_label("Configure Graph", GTK_BOX(vbox));

      {

        hbox = gtk_hbox_new(FALSE, 0);
        
        {
          /*****************************************************************************
           *             Left VBox
           *****************************************************************************/
          GtkWidget *vbox;
          vbox = gtk_vbox_new(FALSE, 4);
          gd->show_container = vbox;

          {
            /******************************************
             *         Show check buttons
             ******************************************/
            create_show_check_button("Show Grid",
                cb_show_grid, qp, vbox, gr->show_grid);
            create_show_check_button("Show Grid Numbers",
                cb_show_grid_numbers, qp, vbox, gr->grid_numbers);
            create_show_check_button("Show Lines",
                cb_show_lines, qp, vbox, 1);
            create_show_check_button("Show Points",
                cb_show_points, qp, vbox, 1);
          }

          {
            /******************************************
             *         X and Y Scales
             ******************************************/
            GtkWidget *combo_box;
            gd->x_scale = combo_box = gtk_combo_box_text_new();
            gtk_combo_box_text_insert_text(GTK_COMBO_BOX_TEXT(combo_box), 0, "Different X Scales");
            gtk_combo_box_text_insert_text(GTK_COMBO_BOX_TEXT(combo_box), 1, "Same X Scales");
            //gtk_combo_box_set_active(GTK_COMBO_BOX(combo_box), 0);
            g_signal_connect(G_OBJECT(combo_box), "changed", G_CALLBACK(cb_same_x_scale), qp);
            gtk_box_pack_start(GTK_BOX(vbox), combo_box, FALSE, FALSE, 0);
            gtk_widget_show(combo_box);

            gd->y_scale = combo_box = gtk_combo_box_text_new();
            gtk_combo_box_text_insert_text(GTK_COMBO_BOX_TEXT(combo_box), 0, "Different Y Scales");
            gtk_combo_box_text_insert_text(GTK_COMBO_BOX_TEXT(combo_box), 1, "Same Y Scales");
            //gtk_combo_box_set_active(GTK_COMBO_BOX(combo_box), 0);
            g_signal_connect(G_OBJECT(combo_box), "changed", G_CALLBACK(cb_same_y_scale), qp);
            gtk_box_pack_start(GTK_BOX(vbox), combo_box, FALSE, FALSE, 0);
            gtk_widget_show(combo_box);
          }

          {
            /********************************************
             *           Color and Font
             ********************************************/
            GtkWidget *table;
            table = gtk_table_new (4, 2, FALSE);
            gtk_table_set_col_spacing (GTK_TABLE (table), 0, 10);
            gtk_table_set_row_spacings(GTK_TABLE (table), 3);
            {
              GtkWidget *label, *picker;
              label = gtk_label_new ("Background Color:");
              gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
              gd->background_color_picker = picker = gtk_color_button_new();
              gtk_color_button_set_title(GTK_COLOR_BUTTON(picker),
                  "Select the Graph Background Color");
              gtk_color_button_set_use_alpha(GTK_COLOR_BUTTON(picker), TRUE);
              gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 0, 1);
              gtk_table_attach_defaults (GTK_TABLE(table), picker, 1, 2, 0, 1);
              g_signal_connect(G_OBJECT(picker), "color-set",
                  G_CALLBACK(cb_background_color), qp);
              gtk_widget_show(label);
              gtk_widget_show(picker);

              label = gtk_label_new("Grid Lines Color:");
              gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
              gd->grid_color_picker = picker = gtk_color_button_new();
              gtk_color_button_set_title(GTK_COLOR_BUTTON(picker),
                  "Select the Graph Grid Color");
              gtk_color_button_set_use_alpha(GTK_COLOR_BUTTON(picker), TRUE);
              gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 1, 2);
              gtk_table_attach_defaults(GTK_TABLE(table), picker, 1, 2, 1, 2);
              g_signal_connect(G_OBJECT(picker), "color-set",
                  G_CALLBACK(cb_grid_color), qp);
              gtk_widget_show(label);
              gtk_widget_show(picker);

              label = gtk_label_new ("Grid Numbers Color:");
              gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
              gd->numbers_color_picker = picker = gtk_color_button_new();
              gtk_color_button_set_title(GTK_COLOR_BUTTON(picker),
                  "Select the Graph Grid Numbers Color");
              gtk_color_button_set_use_alpha(GTK_COLOR_BUTTON(picker), TRUE);
              gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 2, 3);
              gtk_table_attach_defaults(GTK_TABLE(table), picker, 1, 2, 2, 3);
              g_signal_connect(G_OBJECT(picker), "color-set",
                  G_CALLBACK(cb_grid_numbers_color), qp);
              gtk_widget_show(label);
              gtk_widget_show(picker);

              label = gtk_label_new ("Grid Numbers Font:");
              gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
              gd->font_picker = picker = gtk_font_button_new ();
              gtk_font_button_set_title(GTK_FONT_BUTTON(picker),
                  "Select the Graph Grid Numbers Font");
              gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 3, 4);
              gtk_table_attach_defaults(GTK_TABLE(table), picker, 1, 2, 3, 4);
              g_signal_connect(G_OBJECT(picker), "font-set",
                  G_CALLBACK(cb_grid_font), qp);
              gtk_widget_show(label);
              gtk_widget_show(picker);
            }

            gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 0);
            gtk_widget_show(table);
          }


          gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 10);
          gtk_widget_show(vbox);


          /*****************************************************************************
           *             Right VBox
           *****************************************************************************/
          vbox = gtk_vbox_new(FALSE, 2);

          {
            /***********************************************
             *          Int Number Sliders
             ***********************************************/
            gd->line_width_slider = create_slider_input(
                "Plots Line Width", vbox, 1, 20, 200);

            gd->point_size_slider = create_slider_input(
                "Plots Point Size", vbox, 1, 20, 200);

            gd->grid_line_width_slider = create_slider_input(
                "Grid Line Width", vbox, 1, 20, 200);

            gd->grid_x_line_space_slider = create_slider_input(
                "Grid X Line Space", vbox, 30, 600, 2000);

            gd->grid_y_line_space_slider = create_slider_input(
                "Grid Y Line Space", vbox, 30, 600, 2000);
          }

          gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 10);
          gtk_widget_show(vbox);

          /*****************************************************************************/

        }
        
        gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 8);
        gtk_widget_show(hbox);

        make_redraw_button(vbox, qp);
      }


      tab_label = gtk_label_new("Configure Graph");
      gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, tab_label);

      gtk_widget_show(tab_label);
      gtk_widget_show(vbox);
    }
    {
      /*************************************************************************
       *                   Select Plots
       *************************************************************************/

      GtkWidget *vbox, *tab_label, *scrollwin, *label, *hbox;
      GtkAdjustment *xadj, *yadj;

      vbox = gtk_vbox_new(FALSE, 0);

      make_pretty_header_label("Select Channels to Plot or Unplot", GTK_BOX(vbox));

      xadj = gtk_adjustment_new(0,0,0,0,0,0);
      yadj = gtk_adjustment_new(0,0,0,0,0,0);


      hbox = gtk_hbox_new(FALSE, 0);
      label = gtk_label_new("X Channel");
      gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 20);
      gtk_widget_show(label);
      label = gtk_label_new("Y Channel");
      gtk_box_pack_end(GTK_BOX(hbox), label, FALSE, FALSE, 30);
      gtk_widget_show(label);
      gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
      gtk_widget_show(hbox);

      scrollwin = gtk_scrolled_window_new(xadj, yadj);
      {
        GtkWidget *box;
        qp->graph_detail->selector_hbox = box = gtk_hbox_new(FALSE, 2);
        plot_selector_make(qp);
        gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrollwin), box);
        gtk_widget_show(box);
      }
      gtk_box_pack_start(GTK_BOX(vbox), scrollwin, TRUE, TRUE, 8);
      gtk_widget_show(scrollwin);

      //make_redraw_button(vbox, qp);

      gtk_widget_show(vbox);
      tab_label = gtk_label_new("Select Channels to Plot");
      gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, tab_label);
      gtk_widget_show(tab_label);

    }

    {
      /*************************************************************************
       *                   Show Values
       *************************************************************************/

      GtkWidget *vbox;
      GtkWidget *tab_label;

      vbox = gtk_vbox_new(FALSE, 0);

      tab_label = gtk_label_new("Values");
      gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, tab_label);

      gtk_widget_show(tab_label);
      gtk_widget_show(vbox);
    }

  
    gtk_container_add(GTK_CONTAINER(qp->graph_detail->window), notebook);
    gtk_widget_show(notebook);
  }

  qp_qp_graph_detail_init(qp);
}

static
void graph_detail_show(struct qp_qp *qp)
{
  ASSERT(qp);

  if(!qp->graph_detail)
    graph_detail_create(qp);

  qp_qp_graph_detail_init(qp);

  gtk_widget_show(qp->graph_detail->window);
  gtk_window_present(GTK_WINDOW(qp->graph_detail->window));
}

/* We let the view menu be the holder of the state of whether
 * or not the graph_detail should be showing or not, instead
 * of declaring another variable. */
void
cb_view_graph_detail(GtkWidget *w, gpointer data)
{
  struct qp_qp *qp;
  qp = data;

  if(_cb_view_graph_detail_reenter)
    return;
    
  _cb_view_graph_detail_reenter = 1;

  if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(qp->view_graph_detail)))
    graph_detail_show(qp);
  else
    graph_detail_hide(qp);

  _cb_view_graph_detail_reenter = 0;
}

void
cb_graph_detail_show_hide(GtkWidget *w, gpointer data)
{
  struct qp_qp *qp;
  qp = data;

  if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(qp->view_graph_detail)))
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(qp->view_graph_detail), FALSE);
  else
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(qp->view_graph_detail), TRUE);
}


