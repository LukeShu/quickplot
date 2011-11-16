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
#define _GNU_SOURCE

#include <ctype.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>

#include <gtk/gtk.h>

#include "quickplot.h"

#include "config.h"
#include "qp.h"
#include "debug.h"
#include "spew.h"
#include "term_color.h"
#include "list.h"
#include "channel.h"
#include "channel_double.h"


static
int read_ascii(qp_source_t source, FILE *file)
{
  char *line = NULL;
  size_t line_buf_len = 0;
  ssize_t n;
  size_t line_count = 0;
  int (*parse_line)(struct qp_source *source,
      char *line);
  int data_flag = -1;

  /* TODO: check for other types */
  source->value_type = QP_TYPE_DOUBLE;

  switch(source->value_type)
  {
    /* TODO: brake these cases up into
     * other data parsers. */
    case QP_TYPE_DOUBLE:

      parse_line = qp_source_parse_doubles;
      break;

  /* TODO: add other cases */

    default:
      ASSERT(0);
      break;
  }


  do
  {
    /* we seperate the first read because it shows
     * the error case when the file has no data at all. */
    errno = 0;
    n = getline(&line, &line_buf_len, file);

    ++line_count;
    if(n == -1 || errno)
    {
      if(!errno)
      {
        /* end-of-file so it is zero length */
        WARN("getline() read no data in file %s\n",
            source->name);
        QP_WARN("read no data in file %s\n",
            source->name);
      }
      else
      {
        EWARN("getline() read no data in file %s\n",
            source->name);
        QP_EWARN("read no data in file %s\n",
            source->name);

      }
      if(line)
        free(line);
      return 1; /* error */
    }
    data_flag = parse_line(source, line);
  } while(data_flag == 0);

  /* Now we have an least one line of values.
   * We would have returned if we did not. */

  errno = 0;
  while((n = getline(&line, &line_buf_len, file)) > 0)
  {
    ++line_count;
    parse_line(source, line);
    errno = 0;
  }

  if(line)
    free(line);

  /* n == -1 and errno == 0 on end-of-file */
  if((n == -1 && errno != 0) || errno)
  {
#if QP_DEBUG
    if(errno)
      EWARN("getline() failed to read file %s\n",
          source->name);
    else
      WARN("getline() failed to read file %s\n",
          source->name);
#endif
    QP_WARN("failed to read file %s\n", source->name);
    return 1; /* error */
  }

  return 0; /* success */
}

/* returns an allocated string */
static char *unique_name(const char *name)
{
  size_t num = 1, len = 0;
  char *buf = NULL;
  ASSERT(name && name[0]);
  ASSERT(name[strlen(name)-1] != DIR_CHAR);

  if(name[0] == '-' && !name[1])
  {
    name = "stdin";
    /* Sometimes the user wants to know why the
     * program is just hanging.  It is hanging
     * because it is waiting for input from the
     * terminal? */
    QP_NOTICE("Reading stdin\n");
  }


  {
    /* we use just the base name without the directory part */
    char *s;
    s = (char *) &name[strlen(name)-1];
    while(s != name && *s != DIR_CHAR)
      --s;
    if(*s == DIR_CHAR)
      ++s;
    buf = (char *) (name = qp_strdup(s));
  }

  while(1)
  {
    struct qp_source *s;
    for(s=(struct qp_source*) qp_sllist_begin(app->sources);
        s;
        s=(struct qp_source*) qp_sllist_next(app->sources))
      if(strcmp(s->name, buf) == 0)
        break;
    if(s)
    {
      ++num;
      if(buf == name)
        buf = (char*) qp_malloc(len= strlen(buf)+16);
      snprintf(buf, len, "%s (%zu)", name, num);
    }
    else
    {
      if(buf != name)
        free((char*)name);
      return buf;
    }
  }
}


static inline
struct qp_source *
make_source(const char *filename, int value_type)
{
  qp_app_check();
  ASSERT(filename && filename[0]);
  struct qp_source *source;
  source = (struct qp_source *)
    qp_malloc(sizeof(struct qp_source));
  source->name = unique_name(filename);
  source->num_values = 0;
  source->value_type = (value_type)?value_type:QP_TYPE_DOUBLE;
  source->num_channels = 0;
  /* NULL terminated array on channels */
  source->channels = (struct qp_channel **)qp_malloc(
      sizeof(struct qp_channel *));
  *(source->channels) = NULL;
  qp_sllist_append(app->sources, source);

  return source;
}

qp_source_t qp_source_create(const char *filename, int value_type)
{
  struct qp_source *source;
  FILE *file = NULL;

  source = make_source(filename, value_type);

  if(strcmp(filename,"-") == 0)
    file = stdin;

  if(!file)
  {
    errno = 0;
    file = fopen(filename, "r");
  }
  if(!file)
  {
    EWARN("fopen(\"%s\",\"r\") failed\n", filename);
    QP_EWARN("%sFailed to open file%s %s%s%s\n",
        bred, trm, btur, filename, trm);
    goto fail;
  }


  /* TODO: determine file type. Assume ASCII text for now. */

  


  if(read_ascii(source, file))
    goto fail;


  {
    /* remove any channels that have very bad data */
    struct qp_channel **c;
    size_t i = 0, chan_num = 0;
    ASSERT(source->channels);
    c = source->channels;
    while(c[i])
    {
      ASSERT(c[i]->form == QP_CHANNEL_FORM_SERIES);
      if(!is_good_double(c[i]->series.min) ||
          !is_good_double(c[i]->series.max))
      {
        struct qp_channel **n;
        
        qp_channel_destroy(c[i]);

        /* move of all pointers from c[i+1] back one */
        for(n=&c[i]; *n;++n)
          *n = *(n+1);

        /* re-malloc copying and removing one */
        source->channels = 
          qp_realloc(source->channels, sizeof(struct qp_channel *)*
              ((source->num_channels)--));
        
        /* reset c to the next one which is now at the
         * same index */
        c = source->channels;

        QP_NOTICE("removed bad channel number %zu\n", chan_num);
      }
      else
        ++i;
      ++chan_num;
    }
    ASSERT(source->num_channels == i);
  }

  
  if(source->num_channels == 0)
    goto fail;

  
  add_source_buffer_remove_menus(source);

  INFO("created source with %zu sets of values "
      "in %zu channels from file %s\n",
      source->num_values, source->num_channels,
      filename);

  QP_NOTICE("created source with %zu sets of "
      "values in %zu channels from file %s\n",
      source->num_values, source->num_channels,
      filename);

  return source;

fail:

  if(file)
    fclose(file);

  if(source)
    qp_source_destroy(source);

  return NULL;
}


qp_source_t qp_source_create_from_func(
    const char *name, int val_type,
    void (* func)(const void *))
{
  struct qp_source *source;
  source = make_source(name, val_type);

  /* TODO: add code here */

  add_source_buffer_remove_menus(source);
  return source;
}

static inline
void remove_source_menu_item(struct qp_qp *qp,
    struct qp_source *source)
{
  GList *l, *l_alloc;
  l_alloc = (l =
      gtk_container_get_children(GTK_CONTAINER(qp->file_menu)));
  for(l=g_list_first(l); l; l=g_list_next(l))
  {
    /* The menu_item has a pointer to the source
     * in g_object_get_data(). */
    if(g_object_get_data(G_OBJECT(GTK_WIDGET(l->data)),
          "quickplot-source") == (void*) source)
    {
      /* this will remove from file menu */
      gtk_widget_destroy(GTK_WIDGET(l->data)); 
      break;
    }
  }
  if(l_alloc)
    /* no memory leak here */
    g_list_free(l_alloc);
}

/* returns 0 if there were no plots removed */
static inline
int remove_plots_from_graph(struct qp_graph *gr, struct qp_source *source)
{
  struct qp_plot *p;
  int ret = 0;

  p = qp_sllist_begin(gr->plots);
  while(p)
  {
     struct qp_channel **c;
     struct qp_plot *rp = NULL;
     for(c=source->channels; *c; ++c)
       if(qp_channel_equals(p->x, *c) ||
           qp_channel_equals(p->y, *c))
       {
         
          rp = p;
          break;
       }

     p = qp_sllist_next(gr->plots);

     if(rp)
     {
       ASSERT(rp != p);
       /* qp_sllist_remove() works so long as the
        * current plot, p, is not rp, and that
        * should be so since rp is the one before p. */
       qp_sllist_remove(gr->plots, rp, 0);
       qp_plot_destroy(rp, gr);
       ret = 1;
     }
  }
  return ret;
}

/* fix stuff when plots are removed from a graph
 * plots are removed because a source is removed. */
static inline
void fix_or_remove_changed_graph(struct qp_qp *qp, struct qp_graph *gr)
{
  gint pnum;
  ASSERT(qp);
  ASSERT(qp->window);

  if(qp_sllist_length(gr->plots) == 0)
  {
    qp_graph_destroy(gr); /* remove */
    if(qp_sllist_length(qp->graphs) == 0)
    {
      struct qp_sllist_entry *current;
      /* we save and restore the list iterator
       * because qp_graph_create() uses the app qp list. */
      /* TODO: fix this Kludgey List Hack */
      current = app->qps->current;
      qp_graph_create(qp, NULL);
      app->qps->current = current;
    }
    return;
  }

  /* TODO: add rescale fixes due to max and min changes */

  /* fix it by queuing redraw if it's showing */
  pnum = gtk_notebook_get_current_page(GTK_NOTEBOOK(qp->notebook));
  gr->pixbuf_needs_draw = 1;
  if(gtk_notebook_get_nth_page(GTK_NOTEBOOK(qp->notebook), pnum)
      == gr->drawing_area)
    gtk_widget_queue_draw(qp->notebook);
}

void qp_source_destroy(qp_source_t source)
{
  ASSERT(source);
  ASSERT(source->name);
  ASSERT(source->channels);

  if(!source) return;


  { /* remove this source from buffers list from file menu
     * in all main windows (qp) */
    struct qp_qp *qp;
  
    for(qp=qp_sllist_begin(app->qps);
      qp; qp=qp_sllist_next(app->qps))
    {
      if(qp->window)
        remove_source_menu_item(qp, source);
    
      {
        /* remove all plots that use the channels in this source. */
        struct qp_graph *gr;
        gr = qp_sllist_begin(qp->graphs);
        while(gr)
        {
          struct qp_graph *prev_gr;
          prev_gr = gr;
          gr = qp_sllist_next(qp->graphs);

          /* if we remove prev_gr from the qp_sllist in the
           * middle of this iteration it is okay because
           * we are not removing at the current list entry.
           * That is why we check the previous entry and
           * not the current. */
          if(remove_plots_from_graph(prev_gr, source))
            fix_or_remove_changed_graph(qp, prev_gr);
        }

      }
    }
  }

  {
    struct qp_channel **c;
    c = source->channels;
    for(c=source->channels; *c; ++c)
      qp_channel_destroy(*c);
    free(source->channels);
  }

  qp_sllist_remove(app->sources, source, 0);
  
  free(source->name);
  free(source);
}


#ifdef QP_DEBUG
void qp_source_debug_print(struct qp_source *source)
{
  struct qp_channel **c;
  size_t i = 0;
  ASSERT(source);
  DEBUG("\nQP source with %zu sets of values in "
      "%zu channels from file %s\n",
      source->num_values, source->num_channels,
      source->name);

  if(APPEND_ON())
  {
    for(c=source->channels; *c; ++c)
    {
      APPEND("Channel %zu =", i++);
      qp_channel_debug_append(*c);
      APPEND("\n");
    }
  }
}
#endif

