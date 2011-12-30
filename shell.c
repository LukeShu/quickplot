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

/* This file provides a Command line interface (CLI) or Command-line shell
 * for a running quickplot program.  Add a language to this like Ruby.
 *
 * All class objects are containers of parameters.
 * Example
 *        class object: plot
 *        has parameter: line_width
 *        Therefore it's parent graph has line_width which sets and gets
 *        all the plot line_width values.
 *        Therefore it's parent win has line_width which sets and gets
 *        all the graph, plot line_width values.
 *        Therefore it's parent app has line_width which sets and gets
 *        all the win, graph, plot line_width values.
 *
 * Classes:
 *
 *    app
 *           Global parameter settings.  Factory of wins. Factory of sources.
 *           app is a singlet.  You cannot create or destroy app.
 *
 *    source
 *           Source parameter settings. Factory of channels.
 *
 *    channel
 *           Holds the loaded data.
 *
 *    win
 *           Window parameters settings.  Factory of graphs.
 *
 *    graph
 *           Graph parameter settings.  Factory of plots.
 *
 *    plot
 *           Plot parameter settings.
 *
 *
 * Commands:
 *
 *    set
 *           Set class object(s) parameters.
 *
 *    get
 *           Get/display/print parameter(s) for class objects.
 *
 *    create
 *           Create a class object.
 *
 *    destroy
 *           Destroy a class object.
 *
 *
 * Examples of Parameters:
 *
 *    plot::line_width           double
 *    graph::grid                bool (int)             show the grid
 *    graph::grid_x              bool (int)             show the x grid
 *    graph::grid_line_color     double {r, g, b, a}    both x and y
 *    graph::grid_x_line_color   double {r, g, b, a}
 *    win::border                bool (int)             show the window border
 *    win::maximize              bool (int)             window maximized
 *
 *    graph::line_width          doubles                all plots line_width
 *    win::line_width            doubles                all graphs,plots line_width
 *    app::line_width            doubles                all wins,graphs,plots line_width
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <X11/Xlib.h>

#include <gtk/gtk.h>

#include "quickplot.h"

#include "shell.h"
#include "config.h"
#include "qp.h"
#include "debug.h"
#include "list.h"
#include "spew.h"
#include "term_color.h"
#include "shell_common.h"

#ifdef DMALLOC
#  include "dmalloc.h"
#endif


#ifdef HAVE_LIBREADLINE
/* There can only be one readline shell */
static
struct qp_shell *rdln_shell = NULL;
#endif

static
gboolean prepare(GSource *source, gint *timeout)
{
  *timeout = -1; /* block */
  return FALSE;
}

static
gboolean check(GSource *source)
{
  struct qp_shell *sh;
  sh = (struct qp_shell *) source;

  if(sh->fd.revents & G_IO_IN)
    return TRUE;
  return FALSE;
}

void qp_shell_destroy(struct qp_shell *sh)
{
  ASSERT(sh);
  ASSERT(&(sh->gsource));

  if(!sh)
    return;

  if(sh->file_out)
  {
    errno = 0;
    fprintf(sh->file_out, "\nQuickplot Shell exiting\n");
  }

  NOTICE("Quickplot Shell exiting\n");

#ifdef HAVE_LIBREADLINE
  if(sh == rdln_shell)
  {
    /* cleanup readline */
    rl_callback_handler_remove();
    rdln_shell = NULL;
  }
#endif

  if(sh->line)
    free(sh->line);
  if(sh->prompt)
    free(sh->prompt);

  sh->fd.revents = 0;

  g_source_remove_poll(&sh->gsource, &(sh->fd));
  /* g_source_destroy() removes a source from its' GMainContext */
  g_source_destroy(&sh->gsource);
  /* free the gsource object in memory */
  g_source_unref(&sh->gsource);

  if(sh->close_on_exit)
  {
    if(sh->file_in)
      fclose(sh->file_in);
    if(sh->file_out)
      fclose(sh->file_out);
  }

  if(app->op_shell == sh)
    app->op_shell = NULL;

  qp_sllist_remove(app->shells, sh, 0);
}

static inline
void qp_shell_process_command(struct qp_shell *sh, char *line)
{
  fprintf(sh->file_out, "process_command(length=%zu): %s\n", strlen(line), line);

  if(sh->pid != app->pid)
  {
    /* The protocol is: the user writes a one line and then
     * the server responds with any number of lines and ends
     * with "\n" from the last response line plus "END\n"
     * meaning this is the end of the returned data from
     * the last command. */
    fprintf(sh->file_out, "END\n");
    DEBUG("END\n");
  }
  errno = 0;
  if(fflush(sh->file_out))
  {
    QP_EWARN("fflush(fd=%d) failed\n", fileno(sh->file_out));
    EWARN("fflush() failed\n");
  }
}

/* returns 1 if the shell is done
 * returns 0 if not */
static inline
int do_getline(struct qp_shell *sh)
{
  size_t len;

  if(getline(&sh->line, &sh->len, sh->file_in) == -1)
  {
    DEBUG("getline returned -1\n");
    qp_shell_destroy(sh);
    return 1;
  }

  len = strlen(sh->line);
  ASSERT(len > 0);
  /* remove newline '\n' */
  if(sh->line[len-1] == '\n')
    sh->line[len-1] = '\0';
  qp_shell_process_command(sh, sh->line);

  if(sh->out_isatty)
  {
    fprintf(sh->file_out, "%s", sh->prompt);
    fflush(sh->file_out);
  }
  return 0;
}

#ifdef HAVE_LIBREADLINE
static
void readline_handler(char *line)
{
  ASSERT(rdln_shell);
  if(line)
  {
#  ifdef HAVE_READLINE_HISTORY
    if(*line)
      add_history(line);
#  endif
    qp_shell_process_command(rdln_shell, line);
    free(line);
  }
  else
    qp_shell_destroy(rdln_shell);
}
#endif

static
gboolean dispatch(GSource *source, GSourceFunc callback, gpointer data)
{
  struct qp_shell *sh;
  sh = (struct qp_shell *) source;

  do
  {
#ifdef HAVE_LIBREADLINE
    if(sh == rdln_shell)
    {
      rl_callback_read_char();
      if(!rdln_shell)
        break;
    }
    else
#endif
    if(do_getline(sh))
      break;
  }
  while(check_file_in(sh->file_in, 0, 0));

  return TRUE;
}

static
GSourceFuncs source_funcs = { prepare, check, dispatch, NULL, NULL, NULL };


struct qp_shell *qp_shell_create(FILE *file_in, FILE *file_out,
    int close_on_exit, pid_t pid)
{
  struct qp_shell *sh;
  GSource *s;
  gint gid;
  
  ASSERT(file_out);

  if(file_in == NULL)
    file_in = stdin;
  if(file_out == NULL)
    file_out = stdout;

  setlinebuf(file_in);

#if 1
  errno = 0;
  if(fcntl(fileno(file_in), F_SETFL, FNDELAY))
  {
    VASSERT(0, "fcntl(fd=%d, F_SETFL, FNDELAY) failed\n", fileno(file_in));
    QP_EWARN("fcntl(fd=%d, F_SETFL, FNDELAY) failed\n", fileno(file_in));
    return NULL;
  }
#endif

  sh = (struct qp_shell *) g_source_new(&source_funcs, sizeof(*sh));
  sh->fd.fd = fileno(file_in);
  sh->fd.events = G_IO_IN;
  sh->fd.revents = 0;
  sh->file_in = file_in;
  sh->file_out = file_out;
  sh->line = NULL;
  sh->len = 0;
  sh->close_on_exit = close_on_exit;
  sh->pid = pid;
  sh->out_isatty = isatty(fileno(sh->file_out));


  sh->prompt = getenv("QP_PROMPT");
  if(!sh->prompt)
    sh->prompt = getenv("PS2");

  if(sh->prompt)
    sh->prompt = qp_strdup(sh->prompt);
  else
    sh->prompt = qp_strdup("QP> ");

  s = &(sh->gsource);
  VASSERT(s, "g_source_new() failed\n");
  /* Adds source to a GMainContext */
  gid = g_source_attach(s, NULL);
  VASSERT(gid > 0, "g_source_attach() failed\n");
  g_source_add_poll(s, &(sh->fd));

  fprintf(sh->file_out, "\nQuickplot version: %s\n", VERSION);

#ifdef HAVE_LIBREADLINE
  if(sh->out_isatty && !app->op_no_readline)
  {
    fprintf(sh->file_out, "Using readline version: %d.%d\n",
        RL_VERSION_MAJOR, RL_VERSION_MINOR);
    rdln_shell = sh;
    /* setup readline */
    rl_callback_handler_install(sh->prompt, readline_handler);
  }
  else
  {
#endif
#ifndef QP_DEBUG
    if(sh->out_isatty)
      /* printing "Quickplot using getline()" may confuse
       * a user into thinking that their terminal is not
       * using readline when it may indeed be using readline. */
#endif
      fprintf(sh->file_out, "Quickplot using getline()\n");

    if(sh->out_isatty)
      fprintf(sh->file_out, "%s", sh->prompt);
#ifdef HAVE_LIBREADLINE
  }
#endif

  fflush(sh->file_out);
  qp_sllist_append(app->shells, sh);
  DEBUG("\n");
  return sh;
}

