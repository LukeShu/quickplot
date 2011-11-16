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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <unistd.h>
#include <math.h>

#include <sndfile.h>

#include "quickplot.h"

#include "config.h"
#include "debug.h"
#include "spew.h"
#include "list.h"
#include "qp.h"
#include "callbacks.h"
#include "get_opt.h"
#include "channel.h"


union qp_parser
{
  struct
  {
    char *err;
    size_t elen;
    int silent;
  } p1;
  struct
  {
    /* needs_graph = last file loaded and not plotted yet */
    char *needs_graph;
    int got_stdin;
  } p2;
};


/* This is the one globel pointer used to do
 * argument parsing.
 * We allocate it and then free it when we are done
 * with it. */
static
union qp_parser *parser = NULL;



#include "parse_args_utils.h"
#include "parse_1st_pass_funcs.h"
#include "parse_2nd_pass_funcs.h"
#include "parse_args.h"




/* exits on failue */
void qp_getargs_1st_pass(int argc, char **argv)
{
  parser = qp_malloc(sizeof(*parser));

  parser->p1.err = NULL;
  parser->p1.elen = 0;
  parser->p1.silent = 0;


  /* make sure that app exists */
  qp_app_check();

  /* default spew   WARN 3 */
  qp_spew_init(3);

  /* This is the an auto-generated function */
  parse_args_1st_pass(argc, argv);

  if(parser->p1.err)
    print_arg_error();


  if(app->op_pipe == -1)
  {
    /* yes 1    no 0 */
    app->op_pipe = check_stdin();
    /* so now app->op_pipe is a bool */
  }
}


/* exits on failure */
void qp_getargs_2nd_pass(int argc, char **argv)
{
  if(!parser)
  {
    QP_ERROR("qp_getargs_1st_pass() was not called yet"
        " when calling %s()\n", __func__);
    exit(1);
  }

  /* reinitialize parser */
  parser->p2.needs_graph = NULL;
  parser->p2.got_stdin = 0;


  /* This is the an auto-generated function */
  parse_args_2nd_pass(argc, argv);

  /* load stdin file if it needs to be */
  check_load_stdin(0);


  if(parser->p2.needs_graph)
  {
    /* the last file loaded has no graph yet */
    
    ASSERT(qp_sllist_last(app->sources));

    if(app->op_default_graph)
      if(qp_qp_graph_default_source(NULL, (qp_source_t)
          qp_sllist_last(app->sources), NULL))
        exit(1);
  }

  free(parser);

  {
    struct qp_qp *qp;
    qp = qp_sllist_first(app->qps);
    /* There should be at least one qp window */
    if(!qp || !qp->window)
      qp_qp_window(NULL, NULL);
  }

  /* Setup/draw the plots in the tabs */
  g_idle_add_full(G_PRIORITY_LOW + 10, idle_callback, NULL, NULL);
}

