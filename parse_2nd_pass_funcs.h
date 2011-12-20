/* running `./mk_options -2' generates a template of this file.
 * But then we have to edit it too.  */

/* This looks kind of stupid but it makes it very easy
 * to keep track of argument options and it should be
 * pretty efficient given these all short inline functions. */


static inline
void parse_2nd_File(const char *filename)
{
  /* by default pipe is read before the
   * first non-pipe file. */
  check_load_stdin(0);
  load_file(filename);
}

static inline
void parse_2nd_auto_scale(void)
{
  app->op_same_x_scale = -1;
  app->op_same_y_scale = -1;
}

static inline
void parse_2nd_border(void)
{
  app->op_border = TRUE;
}

static inline
void parse_2nd_buttons(void)
{
  app->op_buttons = 1;
}

static inline
void parse_2nd_cairo_draw(void)
{
  app->op_x11_draw = 0;
  if(default_qp)
    default_qp->x11_draw = 0;
}

static inline
void parse_2nd_default_graph(void)
{
  app->op_default_graph = 1;

  if(qp_sllist_last(app->sources) &&
      qp_win_graph_default_source(NULL, (qp_source_t)
          qp_sllist_last(app->sources), NULL))
    exit(1);

  parser->p2.needs_graph = NULL;
}

static inline
void parse_2nd_different_scale(void)
{
  app->op_same_x_scale = 0;
  app->op_same_y_scale = 0;
}

static inline
void parse_2nd_fullscreen(void)
{
  app->op_maximize = 2;
}

static inline
void parse_2nd_gaps(void)
{
  app->op_gaps = 1;
}

static inline
void parse_2nd_grid(void)
{
  app->op_grid = 1;
}

static inline
void parse_2nd_grid_numbers(void)
{
  app->op_grid_numbers = 1;
}

static inline
void parse_2nd_gui(void)
{
  app->op_menubar = 1;
  app->op_buttons = 1;
  app->op_tabs = 1;
  app->op_statusbar = 1;
}

static inline
void parse_2nd_labels(void)
{
  app->op_labels = 1;
}

static inline
void parse_2nd_maximize(void)
{
  app->op_maximize = 1;
}

static inline
void parse_2nd_menubar(void)
{
  app->op_menubar = 1;
}

static inline
void parse_2nd_new_window(void)
{
  app->op_new_window = 1;
}

static inline
void parse_2nd_no_border(void)
{
  app->op_border = 0;
}

static inline
void parse_2nd_no_buttons(void)
{
  app->op_buttons = 0;
}

static inline
void parse_2nd_no_default_graph(void)
{
  app->op_default_graph = 0;
}

static inline
void parse_2nd_no_fullscreen(void)
{
  if(app->op_maximize == 2)
    app->op_maximize = 0;
}

static inline
void parse_2nd_no_gaps(void)
{
  app->op_gaps = 0;
}

static inline
void parse_2nd_no_grid(void)
{
  app->op_grid = 0;
}

static inline
void parse_2nd_no_grid_numbers(void)
{
  app->op_grid_numbers = 0;
}

static inline
void parse_2nd_no_gui(void)
{
  app->op_menubar = 0;
  app->op_buttons = 0;
  app->op_tabs = 0;
  app->op_statusbar = 0;
}

static inline
void parse_2nd_no_labels(void)
{
  app->op_labels = 0;
}

static inline
void parse_2nd_no_linear_channel(void)
{
  if(app->op_linear_channel)
  {
    qp_channel_destroy(app->op_linear_channel);
    app->op_linear_channel = NULL;
  }
}

static inline
void parse_2nd_no_lines(void)
{
  app->op_lines = 0;
}

static inline
void parse_2nd_no_menubar(void)
{
  app->op_menubar = 0;
}

static inline
void parse_2nd_no_maximize(void)
{
  if(app->op_maximize == 1)
    app->op_maximize = 0;
}

static inline
void parse_2nd_no_new_window(void)
{
  app->op_new_window = 0;
}

static inline
void parse_2nd_no_points(void)
{
  app->op_points = 0;
}

static inline
void parse_2nd_no_shape(void)
{
  app->op_shape = 0;
}

static inline
void parse_2nd_no_statusbar(void)
{
  app->op_statusbar = 0;
}

static inline
void parse_2nd_no_tabs(void)
{
  app->op_tabs = 0;
}

static inline
void parse_2nd_pipe(void)
{
  check_load_stdin(1);
}

static inline
void parse_2nd_points(void)
{
  app->op_points = 1;
}

static inline
void parse_2nd_read_pipe_here(void)
{
  check_load_stdin(1);
}

static inline
void parse_2nd_same_scale(void)
{
  app->op_same_x_scale = 1;
  app->op_same_y_scale = 1;
}

static inline
void parse_2nd_shape(void)
{
  app->op_shape = 1;
}

static inline
void parse_2nd_statusbar(void)
{
  app->op_statusbar = 1;
}

static inline
void parse_2nd_tabs(void)
{
  app->op_tabs = 1;
}

static inline
void parse_2nd_x11_draw(void)
{
  app->op_x11_draw = 1;
  if(default_qp)
    default_qp->x11_draw = 1;
}


static inline
void parse_2nd_background_color(char *arg, int argc, char **argv, int *i)
{
  get_color(&app->op_background_color, arg);
}

static inline
void parse_2nd_file(char *arg, int argc, char **argv, int *i)
{
  parse_2nd_File(arg);
}

static inline
void parse_2nd_geometry(char *arg, int argc, char **argv, int *j)
{
  /* We use this for the next new main window. */
  int n[4], count = 0, x_count = -1;
  int w, h;
  char *endptr, *s;
  endptr = s = arg;

  while(1)
  {
    long val;

    val = strtol(s, &endptr, 10);
    if(s == endptr || !endptr ||
      val == LONG_MAX || val == LONG_MIN)
    {
      DEBUG("bad option: %s='%s'\n", "--geometry", arg);
      QP_ERROR("option has bad integer number: %s='%s'\n",
          "--geometry", arg);
      exit(1);
    }
    if(*s == '-' || *s == '0')
      /* mark a "-0" position as in --geometry=600x700-0+0 */
      n[count++] = INT_MIN;
    else
      n[count++] = val;

    if(!(*endptr))
      break;

    s = endptr;
    while(*s && (*s < '0' || *s > '9') && *s != '-' && *s != '+')
    {
      if(*s == 'x' || *s == 'X')
        /* this is where the 'x' is */
        x_count = count;
      ++s;
    }
    if(!(*s))
      break;
  }
  
  if(
      !(count == 2 && (x_count == 1 || x_count == -1))
       &&
      !(count == 4 && (x_count == 1 || x_count == 3))
    )
  {
    DEBUG("bad option: %s='%s' count=%d x_count=%d\n",
        "--geometry", arg, count, x_count);
    QP_ERROR("bad option: %s='%s'\n", "--geometry", arg);
    exit(1);
  }

  if(x_count == 1)
    {
      w = n[0];
      h = n[1];
    }
  else if(x_count == 3)
    {
      w = n[2];
      h = n[3];
    }

  if(x_count == -1 || x_count == 3)
    {
      app->op_geometry.x = n[0];
      app->op_geometry.y = n[1];
    }
  else if(count == 4)
    {
      app->op_geometry.x = n[2];
      app->op_geometry.y = n[3];
    }

  if(w < 1 || h < 1)
  {
    QP_ERROR("bad option: %s='%s'\n", "--geometry", arg);
    exit(1);
  }

  if(app->root_window_width < 1)
    qp_get_root_window_size();

  if(w > app->root_window_width)
    w = app->root_window_width;
  if(h > app->root_window_height)
    h = app->root_window_height;

  if(w == app->root_window_width && h == app->root_window_height)
    /* We do not want to set app->op_geometry if it is
     * full screen so that they can still toggle out of
     * full screen if they choose to. */
    app->op_maximize = 2; /* Fullscreen */
  else
  {
    app->op_geometry.width = w;
    app->op_geometry.height = h;
    app->op_maximize = 0;
  }

  DEBUG("got --geometry=%dx%d%+d%+d app->op_maximize=%d\n", 
        w, h,
        app->op_geometry.x, app->op_geometry.y,
        app->op_maximize);
}

static inline
void graph_plots(ssize_t *x, ssize_t *y, size_t len)
{
  ASSERT(len);
  if(qp_win_graph(NULL, x, y, len, NULL))
    exit(1);

  free(x);
  free(y);
  parser->p2.needs_graph = 0;
}

static inline
void parse_2nd_graph(char *arg, int argc, char **argv, int *i)
{
  ssize_t  *x = NULL, *y = NULL, max_channel_num = -1;
  size_t len = 0;
  struct qp_source *s;

  if(!qp_sllist_last(app->sources))
  {
    ERROR("\n");
    QP_ERROR("got option --graph='%s' but have no files read yet\n", arg);
    exit(1);
  }

  for(s=qp_sllist_begin(app->sources);s; s=qp_sllist_next(app->sources))
    max_channel_num += s->num_channels;

  get_plot_option(arg, &x, &y, &len, "--graph", 0, max_channel_num);
  graph_plots(x, y, len);
}

static inline
void parse_2nd_graph_file(char *arg, int argc, char **argv, int *i)
{
  ssize_t  *x = NULL, *y = NULL, offset = 0, j;
  size_t len = 0;
  struct qp_source *s, *last_s;

  last_s = qp_sllist_last(app->sources);
  if(!last_s)
  {
    ERROR("\n");
    QP_ERROR("got option --graph-file='%s' but have no files read yet\n", arg);
    exit(1);
  }

  for(s=qp_sllist_begin(app->sources);s != last_s; s=qp_sllist_next(app->sources))
    offset += s->num_channels;

  get_plot_option(arg, &x, &y, &len, "--graph-file",
      -offset, last_s->num_channels - 1);

  if(!len)
  {
    QP_ERROR("bad option --graph-file=\"%s\"\n", arg);
    exit(1);
  }

  for(j=0;j<len;++j)
  {
    x[j] += offset;
    y[j] += offset;
  }
  
  graph_plots(x, y, len);
}

static inline
void parse_2nd_grid_font(char *arg, int argc, char **argv, int *i)
{
  ASSERT(app->op_grid_font);
  if(app->op_grid_font)
    free(app->op_grid_font);
  app->op_grid_font = qp_strdup(arg);
}

static inline
void parse_2nd_grid_line_width(char *arg, int argc, char **argv, int *i)
{
  app->op_grid_line_width = get_long(arg, 1, 101, "--grid-line-width");
}

static inline
void parse_2nd_grid_line_color(char *arg, int argc, char **argv, int *i)
{
  get_color(&app->op_grid_line_color, arg);
}

static inline
void parse_2nd_grid_text_color(char *arg, int argc, char **argv, int *i)
{
  get_color(&app->op_grid_text_color, arg);
}

static inline
void parse_2nd_grid_x_space(char *arg, int argc, char **argv, int *i)
{
  app->op_grid_x_space = get_long(arg, 10, 10000000, "--grid-x-space");
}

static inline
void parse_2nd_grid_y_space(char *arg, int argc, char **argv, int *i)
{
  app->op_grid_y_space = get_long(arg, 10, 10000000, "--grid-y-space");
}

static inline
void parse_2nd_label_separator(char *arg, int argc, char **argv, int *i)
{
  ASSERT(app->op_label_separator);
  free(app->op_label_separator);
  app->op_label_separator = qp_strdup(arg);
}

static inline
void parse_2nd_line_width(char *arg, int argc, char **argv, int *i)
{
  app->op_line_width = get_long(arg, 1, 101, "--line-width");
}

static inline
void parse_2nd_linear_channel(char *arg, int argc, char **argv, int *i)
{
  double start, step;

  if(app->op_linear_channel)
  {
    qp_channel_destroy(app->op_linear_channel);
    app->op_linear_channel = NULL;
  }

  parse_linear_channel(1, arg, argc, argv, i, &start, &step);

  app->op_linear_channel = qp_channel_linear_create(start, step);
}

static inline
void parse_2nd_lines(char *arg, int argc, char **argv, int *i)
{
  app->op_lines = get_yes_no_auto_int(arg, "--lines");
}

static inline
void parse_2nd_number_of_plots(char *arg, int argc, char **argv, int *i)
{
  app->op_number_of_plots = get_long(arg, 1, INT_MAX - 10,
      "--number-of-plots");
}

static inline
void parse_2nd_point_size(char *arg, int argc, char **argv, int *i)
{
  if(!strncasecmp(arg, "AUTO", 4))
    app->op_point_size = -1;
  else
    app->op_point_size = get_long(arg, 1, 101, "--point-size");
}

static inline
void parse_2nd_same_x_scale(char *arg, int argc, char **argv, int *i)
{
  app->op_same_x_scale = get_yes_no_auto_int(arg, "--same-x-scale");
}

static inline
void parse_2nd_same_y_scale(char *arg, int argc, char **argv, int *i)
{
  app->op_same_y_scale = get_yes_no_auto_int(arg, "--same-y-scale");
}

static inline
void parse_2nd_skip_lines(char *arg, int argc, char **argv, int *i)
{
  app->op_skip_lines = get_long(arg, 0, INT_MAX - 10, "--skip-lines");
}

