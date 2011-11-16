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

#include <gdk/gdkx.h>

#include "quickplot.h"

#include "config.h"
#include "debug.h"
#include "list.h"
#include "channel.h"
#include "channel_double.h"
#include "qp.h"
#include "plot.h"
#include "color_gen.h"

static inline
void free_x11_colors(struct qp_plot *p, struct qp_graph *gr)
{
  ASSERT(gr->x11); 

  if(gr->x11)
  {
    if(!gr->x11->dsp)
      gr->x11->dsp = gdk_x11_get_default_xdisplay();

    XFreeColors(gr->x11->dsp, DefaultColormap(gr->x11->dsp,
          DefaultScreen(gr->x11->dsp)), &p->l.x, 1, 0);
    XFreeColors(gr->x11->dsp, DefaultColormap(gr->x11->dsp,
          DefaultScreen(gr->x11->dsp)), &p->p.x, 1, 0);
  }
}

static inline
void make_x11_colors(struct qp_plot *p, struct qp_graph *gr)
{
  ASSERT(gr->x11);

  if(gr->x11)
  {
    /* Turn the cairo rgba colors into XColors */
    int i;
    struct qp_color *c[2];

    c[0] = &p->p;
    c[1] = &p->l;
    if(!gr->x11->dsp)
      gr->x11->dsp = gdk_x11_get_default_xdisplay();

    for(i=0;i<2;++i)
    {
      XColor x;
      x.pixel = 0;

      x.red   = 65535 * c[i]->c.r;
      x.green = 65535 * c[i]->c.g;
      x.blue  = 65535 * c[i]->c.b;
      x.flags = 0;

      XAllocColor(gr->x11->dsp, DefaultColormap(gr->x11->dsp,
            DefaultScreen(gr->x11->dsp)), &x);
      ASSERT(x.pixel);
      c[i]->x = x.pixel;
    }
  }
}


void qp_plot_set_cairo_draw_mode(struct qp_plot *p, struct qp_graph *gr)
{
  free_x11_colors(p, gr);
}

void qp_plot_set_x11_draw_mode(struct qp_plot *p, struct qp_graph *gr)
{
  make_x11_colors(p, gr);
}

qp_plot_t qp_plot_create(qp_graph_t gr,
      qp_channel_t x, qp_channel_t y, const char *name,
      double xmin, double xmax, double ymin, double ymax)
{
  struct qp_plot *p;
  size_t num_points = (size_t) -1;
  ASSERT(gr);
  ASSERT(x);
  ASSERT(y);
  ASSERT(name);

  p = (struct qp_plot*) qp_malloc(sizeof(*p));
  qp_sllist_append(gr->plots, p);
  p->name = qp_strdup(name);


  /* get default point and line colors */
  qp_color_gen_next(gr->color_gen, &p->p.c.r, &p->p.c.g, &p->p.c.b, -1);
  qp_color_gen_next(gr->color_gen, &p->l.c.r, &p->l.c.g, &p->l.c.b, -1);
  p->p.c.a = 0.95;
  p->l.c.a = 0.9;

  if(gr->x11)
    make_x11_colors(p, gr);


  switch(x->form)
  {
    case QP_CHANNEL_FORM_SERIES:
      {
        if(xmax < xmin)
        {
          xmin = x->series.min;
          xmax = x->series.max;
        }
        p->x = qp_channel_series_create(x, 0);
        p->x_is_reading = qp_channel_series_is_reading;

        switch(x->value_type)
        {
          case QP_TYPE_DOUBLE:
            /* now these functions will not be inlined, oh well */
            p->channel_x_begin = qp_channel_series_double_begin;
            p->channel_x_end = qp_channel_series_double_end;
            p->channel_x_next = qp_channel_series_double_next;
            p->channel_x_prev = qp_channel_series_double_prev;
            p->channel_series_x_index = qp_channel_series_double_index;
            p->channel_series_x_get_index = qp_channel_series_double_get_index;
            break;
          default:
            VASSERT(0, "write more code here");
            break;
        }
      }
      break;
    /* TODO: add more cases */
    default:
      VASSERT(0, "write more code here");
      break;
  }

  switch(y->form)
  {
    case QP_CHANNEL_FORM_SERIES:
      {
        if(ymax < ymin)
        {
          ymin = y->series.min;
          ymax = y->series.max;
        }
        p->y = qp_channel_series_create(y, 0);
        p->y_is_reading = qp_channel_series_is_reading;
  
        switch(y->value_type)
        {
          case QP_TYPE_DOUBLE:
            /* now these functions will not be inlined, oh well */
            p->channel_y_begin = qp_channel_series_double_begin;
            p->channel_y_end = qp_channel_series_double_end;
            p->channel_y_next = qp_channel_series_double_next;
            p->channel_y_prev = qp_channel_series_double_prev;
            p->channel_series_y_index = qp_channel_series_double_index;
            break;
          default:
            VASSERT(0, "write more code here");
            break;
        }
      }
      break;
    /* TODO: add more cases */
    default:
      VASSERT(0, "write more code here");
      break;
  }

  /* find the number of points if we can */
  if(p->x->form == QP_CHANNEL_FORM_SERIES)
      num_points = qp_channel_series_length(p->x);
  if(p->y->form == QP_CHANNEL_FORM_SERIES)
  {
    size_t len;
    len = qp_channel_series_length(p->y);
    if(len < num_points)
      num_points = len;
  }

  if(app->op_lines == -1)
  {
    if(num_points > 1000000)
      p->lines = 0;
    else
      p->lines = 1;
  }
  else
    p->lines = app->op_lines;


  p->points = app->op_points; 


  if(app->op_line_width != -1)
    p->line_width = app->op_line_width;
  else /* AUTO line_width */
  {
    if(num_points > 100000)
      p->line_width = 2;
    else if(num_points > 100)
      p->line_width = 4;
    else if(num_points > 10)
      p->line_width = 6;
    else
      p->line_width = 8;

    if((gr->grid_line_width%2))
      ++(p->line_width);
  }

  if(app->op_point_size != -1)
    p->point_size = app->op_point_size;
  else
  {
    if(num_points > 1000000)
      p->point_size = 2;
    else if(num_points > 10000)
      p->point_size = 4;
    else if(num_points > 100)
      p->point_size = 6;
    else if(num_points > 10)
      p->point_size = 8;
    else
      p->point_size = 10;

    if((gr->grid_line_width%2))
      ++(p->point_size);
  }


  p->xscale0 = 1.0/(xmax - xmin);
  p->xshift0 = -xmin/(xmax - xmin);

  p->yscale0 = 1.0/(ymax - ymin);
  p->yshift0 = -ymin/(ymax - ymin);

  p->xscale = 1.0;
  p->xshift = 0.0;

  p->yscale = 1.0;
  p->yshift = 0.0;

  return p;
}

 
void qp_plot_destroy(qp_plot_t plot, struct qp_graph *gr)
{
  ASSERT(plot);
  ASSERT(plot->name);

  if(plot)
  {
    /* if the channels are series we must free
     * resources */
    if(plot->x->form == QP_CHANNEL_FORM_SERIES)
      qp_channel_destroy(plot->x);
    if(plot->y->form == QP_CHANNEL_FORM_SERIES)
      qp_channel_destroy(plot->y);

    /* If using X11 to draw we need to free the X11 colors */
    if(gr->x11)
      free_x11_colors(plot, gr);
  
    free(plot->name);
    free(plot);
  }
}

