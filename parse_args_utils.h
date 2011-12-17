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

#include <locale.h>

static inline
void print_text(const char *filename)
{
  int fd;
  char buf[1024];
  ssize_t n, i;
  fd = qp_find_doc_file(filename, NULL);
  if(fd == -1) exit(1);

  while((n = read(fd, buf, 1024)) > 0)
  {
    while(n)
    {
      i = write(STDOUT_FILENO, buf, n);
      if(i < 1)
        exit(1);
      n -= i;
    }
  }
  exit(0);
}

static inline
void version(void)
{
  printf("%s\n", VERSION);
  exit(0);
}


static
void print_arg_error(void)
{
  QP_ERROR("Bad arguments:\n%s\n", parser->p1.err);
  exit(1);
}


#define ERR_LEN (1024*8)

/* We do not bother using any memory unless there is a
 * fatal parsing error.
 * We accumulate all command line error messages and
 * when print them, saving the user time. */
static
void add_error(const char *fmt, ...)
{
  va_list ap;
  ssize_t n;
  char *err;
  size_t *len;

  len = &(parser->p1.elen);

  if(parser->p1.err == NULL)
  {
    parser->p1.err = qp_malloc(ERR_LEN);
    *len = 0;
  }

  err = parser->p1.err;


  va_start(ap, fmt);
  n = vsnprintf(&((err)[*len]), ERR_LEN-(*len), fmt, ap);
  va_end(ap);

  if(n <= 0)
  {
    QP_ERROR("internal parsing error\n");
    exit(1);
  }
  *len += n;

  if(*len >= ERR_LEN)
    print_arg_error();
}

static inline
void check_color_opt(const char *long_opt, const char *arg)
{
  /* This is just to test for color parse error */
  GdkRGBA rgba;
  if(!gdk_rgba_parse(&rgba, arg))
    add_error("for argument option %s=\"%s\"\n"
	"  failed to parse string "
	"\"%s\" to a color\n", long_opt,
        arg, arg);
#ifdef QP_DEBUG
  else
    DEBUG("opt %s=\"%s\"  => color=rgba(%g,%g,%g,%g)\n",
        long_opt, arg, rgba.red, rgba.green, rgba.blue, rgba.alpha);
#endif
}

/* returns 0 for not read ready
 * returns 1 for read ready */
static
int check_stdin(void)
{
  /* Check for read on stdin */
  long usec;
  struct timeval tv;
  fd_set fds;
  int ret;
  tv.tv_sec = 0;
  tv.tv_usec = 400000;
  usec = tv.tv_usec;

  FD_ZERO(&fds);
  FD_SET(STDIN_FILENO, &fds);
  errno = 0;
  ret = select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
  if(ret > 0)
  {
    QP_NOTICE("Reading stdin\n");
    return 1;
  }
  if(ret == 0)
  {
    QP_INFO("Nothing to read on stdin in %ld milli seconds\n", usec/1000);
    return 0;
  }
    
  QP_NOTICE("Error checking data on stdin: errno=%d: %s\n",
      errno, strerror(errno));
  return 0;
}

static inline
void parse_linear_channel(int pass, const char *arg,
    int argc, char **argv, int *i, double *start, double *step)
{
  char *s;
  int err = 0, j = 0;
  double val[2];

  if(start)
    *start = 0;
  if(step)
    *step = 1;

  if(arg == NULL || arg[0] == '\0')
    goto ret;

  s = (char *) arg;

  while(1)
  {
    char *endptr = NULL;
    struct lconv *lc;

    errno = 0;
    val[j] = strtod(s, &endptr);
    if(errno || s == endptr)
    {
      if(argv[*i - 1] != arg)
      {
        /* The argument is part of the option like:
         * '--linear-channel=0 1' or -l'0 3'.
         * arg is not part of another option so
         * this is an error */
        err = 1;
        break;
      }
      else
      {
        /* The optional arg is not ours */
        *i -= 1;
        break;
      }
    }

    if(j == 1)
    {
      if(start)
        *start = val[0];
      if(step)
        *step = val[1];
      break;
    }

    lc = localeconv();

    s = endptr;
    /* go to the next number */
    while(*s && (*s < '0' || *s > '9')
        && *s != '+' && *s != '-' && *s != 'e'
        && *s != 'E' && strcmp(s, lc->decimal_point))
      ++s;
  
    if(!(*s))
    {
      if(start)
        *start = val[0];
      break;
    }

    ++j;
  }
  

  if(err && pass == 0)
  {
    add_error("bad option argument --linear-channel='%s'\n", arg);
    return;
  }

  /* we should not get an error on the 2nd pass */
  ASSERT(!err);

  if(err)
  {
    QP_ERROR("bad option argument --linear-channel='%s'\n", arg);
    exit(1);
  }

ret:

  if(start && step)
    DEBUG("got option --linear-channel with start=%.22g step=%.22g\n",
        *start, *step);
}

static
long get_plot_num(const char *s, char **endptr)
{
  while(*s && *s < '0' && *s > '9')
    ++s;
  if(!(*s))
    return INT_MAX; /* got nothing more */

  long val;
  errno = 0;
  val = strtol(s, endptr, 10);

  if(*endptr == s)
    return INT_MAX;

  if((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
      || (errno != 0 && val == 0))
    return INT_MAX; /* got error */

  return val;
}

static
void load_file(const char *filename)
{
  if(parser->p2.needs_graph && app->op_default_graph)
  {
    ASSERT(qp_sllist_last(app->sources));

    if(qp_qp_graph_default_source(NULL, (qp_source_t)
          qp_sllist_last(app->sources), NULL))
      exit(1);

    parser->p2.needs_graph = NULL;
  }

  if(!strcmp(filename, "-") &&
      (parser->p2.got_stdin || !app->op_pipe))
    return;   
        

  if(!qp_source_create(filename, QP_TYPE_UNKNOWN))
     exit(1);

  parser->p2.needs_graph = (char *) filename;

  if(!strcmp(filename, "-"))
    parser->p2.got_stdin = 1;
}

static inline
void check_load_stdin(int here)
{
  if(!parser->p2.got_stdin &&
      (!app->op_read_pipe_here || here)
       && app->op_pipe)
  {
    /* The loading of stdin cannot be done until before
     * the first file or plot option that is --plot,
     * --plot-file, --file, or FILE, so that we are
     * may catch any file reading options 
     * that may effect the reading of the pipe,
     * that may be in arguments before those. */
    load_file("-");
  }
}

/* Returns 1 on success 0 on failure
 *
 * x and y must be freed
 *
 * parsing  --plot  '0 1 0 2 0 3'
 * or --plot-file '0 1 0 2 0 3'
 *
 * str="0 1 0 2 0 3" */
static inline
void get_plot_option(const char *str, ssize_t **x, ssize_t **y,
    size_t *num_plots, const char *opt, ssize_t min_chan, ssize_t max_chan)
{
  char *endptr;
  char *s;
  long nx, ny;
  int i = 1;

  ASSERT(x);
  ASSERT(y);
  ASSERT(num_plots);
  
  check_load_stdin(0);

  if(!qp_sllist_last(app->sources))
  {
    QP_ERROR("No files loaded yet, bad option %s='%s'\n",
         opt, str);
    exit(1);
  }

  s = (char *) str;

  nx = get_plot_num(s, &endptr);
  if(nx < min_chan || nx > max_chan) goto fail;
  s = endptr;
  ny = get_plot_num(s, &endptr);
  if(ny < min_chan || ny > max_chan) goto fail;
  s = endptr;

  if(x && y)
  {
    *x = qp_malloc(sizeof(ssize_t)*(strlen(s)+1)/2);
    *y = qp_malloc(sizeof(ssize_t)*(strlen(s)+1)/2);
    (*x)[0] = nx;
    (*y)[0] = ny;
  }

  while(*s)
  {
    nx = get_plot_num(s, &endptr);
    if(nx < min_chan || nx > max_chan) goto fail;
    (*x)[i] = nx;
    s = endptr;
    ny = get_plot_num(s, &endptr);
    if(ny < min_chan || ny > max_chan) goto fail;
    s = endptr;
    (*y)[i++] = ny;
  }

  *num_plots = i;

#ifdef QP_DEBUG
  DEBUG("got %s='", opt);
  if(*num_plots > 0)
    APPEND("%zu %zu", (*x)[0], (*y)[0]);
  for(i=1; i<(*num_plots); ++i)
    APPEND(" %zu %zu", (*x)[i], (*y)[i]);
  APPEND("\n");
#endif

  return;

fail:

  ERROR("got bad option %s='%s'\n", opt, str);
  QP_ERROR("got bad option %s='%s'\n", opt, str);
  exit(1);
  
  if(x && *x)
    free(*x);
  if(y && *y)
    free(*y);
}

/* This should not fail given we checked colors in the 1st pass */
static inline
void get_color(struct qp_colora *c, const char *str)
{
  GdkRGBA rgba;
  gboolean ret;
  ret = gdk_rgba_parse(&rgba, str);
  VASSERT(ret,"failed to parse color string\n");
  if(!ret)
  {
    QP_ERROR("failed to parse color string %s\n", str);
    exit(1);
  }

  c->r = rgba.red;
  c->g = rgba.green;
  c->b = rgba.blue;
  c->a = rgba.alpha;
}

/* We flip back the tab and wait for the drawing to
 * finish and then flip back to the next tab and
 * so on, until we are at the first tab. */
static
gboolean startup_idle_callback(gpointer data)
{
  static struct qp_qp *qp = NULL;
  struct qp_graph *first_graph;

  if(!qp)
  {
    qp = qp_sllist_first(app->qps);
    ASSERT(qp);
    ASSERT(qp->window);
    ASSERT(qp->initializing);
  }
  else if(qp->initializing == 2)
  {
    /* A user wants to delete this window, maybe
     * before we finished drawing all the tabs. */
    struct qp_qp *q;
    q = qp_sllist_begin(app->qps);
    while(q && q != qp)
      q = qp_sllist_next(app->qps);
    q = qp_sllist_next(app->qps);
    /* q == NULL or the next qp */
    qp_qp_destroy(qp);
    qp = q;
    if(!qp)
      return FALSE;
    
    ASSERT(qp->initializing);
  }

  first_graph = qp_sllist_first(qp->graphs);
  ASSERT(first_graph);

  if(first_graph == qp->current_graph)
  {
    /* we are at the first tab */
    /* see if there are more main windows */
    struct qp_qp *q;
    q = qp_sllist_begin(app->qps);
    while(q && q != qp)
      q = qp_sllist_next(app->qps);

    /* done initializing */
    qp->initializing = 0;

    qp = qp_sllist_next(app->qps);
    if(qp)
    {
      ASSERT(qp->initializing);
      /* now check that all the tabs have
       * been flipped in this qp */
      return startup_idle_callback(NULL);
    }
    return FALSE; /* remove this idle callback */
  }

  /* We flip back the tab */
  gtk_notebook_prev_page(GTK_NOTEBOOK(qp->notebook));

  /* call this when we are idle again.  Hopefully
   * after drawing the graph. */
  return TRUE;
}

static inline
int get_yes_no_auto_int(const char *arg, const char *option)
{
  if(!strncasecmp(arg, "no", 1))
    return 0;
  if(!strncasecmp(arg, "yes", 1))
    return 1;
  if(!strncasecmp(arg, "auto", 1))
    return -1;

  QP_ERROR("bad option: %s='%s'\n", option, arg);
  exit(1);

  return 1;
}

static inline
long get_long(const char *s, long min, long max, const char *opt)
{
  char *endptr = NULL;
  long val = 0;
  val = strtol(s, &endptr, 10);
  if(s == endptr || !endptr ||
      val == LONG_MAX || val == LONG_MIN)
  {
    QP_ERROR("option has bad integer number: %s='%s'\n", opt, s);
    exit(1);
  }
  if(val < min)
    return min;
  if(val > max)
    return max;

  return val;
}


