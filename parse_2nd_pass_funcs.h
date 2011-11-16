/* running `./mk_options -2' generates a template of this file.
 * But then we have to edit it too.  */

/* This looks kind of stupid but it makes it very easy
 * to keep track of argument options and it should be
 * pretty efficient given these all short inline functions. */


static inline
void parse_2nd_File(const char *filename)
{
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
}

static inline
void parse_2nd_default_graph(void)
{
  app->op_default_graph = 1;

  if(qp_sllist_last(app->sources) &&
      qp_qp_graph_default_source(NULL, (qp_source_t)
          qp_sllist_last(app->sources), NULL))
    exit(1);

  parser->p2.needs_graph = NULL;
}

static inline
void parse_2nd_different_scales(void)
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
void parse_2nd_geometry(char *arg, int argc, char **argv, int *i)
{
  /* We use this for the next new main window. */
  long l;
  l = get_long(arg, - INT_MAX + 10, INT_MAX - 10, INT_MAX);

  if(l == INT_MAX)
    app->op_geometry = NULL;
  else
    app->op_geometry = qp_strdup(arg); 
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
  app->op_grid_line_width = get_long(arg, 1, 101, 3);
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
  app->op_grid_x_space = get_long(arg, 10, 10000000, 220);
}

static inline
void parse_2nd_grid_y_space(char *arg, int argc, char **argv, int *i)
{
  app->op_grid_y_space = get_long(arg, 10, 10000000, 190);
}

static inline
void parse_2nd_label_separator(char *arg, int argc, char **argv, int *i)
{
  app->op_label_separator = arg;
}

static inline
void parse_2nd_line_width(char *arg, int argc, char **argv, int *i)
{
  app->op_line_width = get_long(arg, 1, 101, 3);
}

static inline
void parse_2nd_linear_channel(char *arg, int argc, char **argv, int *i)
{
  if(app->op_linear_channel)
  {
    qp_channel_destroy(app->op_linear_channel);
    app->op_linear_channel = NULL;
  }

  ERROR("Need to add linear channel code here");
}

static inline
void parse_2nd_lines(char *arg, int argc, char **argv, int *i)
{
  if(arg)
    app->op_lines = get_yes_no_auto_int(arg, argv, i);
  else
    app->op_lines = 1;
}

static inline
void parse_2nd_number_of_plots(char *arg, int argc, char **argv, int *i)
{
  app->op_number_of_plots = get_long(arg, 1, INT_MAX - 10, NUMBER_OF_PLOTS);
}

static inline
void parse_2nd_plot(char *arg, int argc, char **argv, int *i)
{
  size_t  *x = NULL, *y = NULL, len = 0;

  if(!qp_sllist_last(app->sources))
  {
     ERROR("No files loaded, bad option --plot-file='%s'\n",
         arg);
    exit(1);
  }

  get_plot_option(arg, &x, &y, &len, "--plot", 1);
  ASSERT(len);
  if(qp_qp_graph(NULL, x, y, len, NULL))
    exit(1);

  parser->p2.needs_graph = 0;
}

static inline
void parse_2nd_plot_file(char *arg, int argc, char **argv, int *i)
{
  size_t  *x = NULL, *y = NULL, len = 0, offset = 0, j;
  struct qp_source *s, *last_s;


  if(!qp_sllist_last(app->sources))
  {
     ERROR("No files loaded, bad option --plot-file='%s'\n",
         arg);
    exit(1);
  }

  get_plot_option(arg, &x, &y, &len, "--plot", 1);

  if(!len)
  {
    QP_ERROR("bad option --plot-file=\"%s\"\n", arg);
    exit(1);
  }

  last_s = qp_sllist_last(app->sources);
  VASSERT(last_s, "got option --plot-file but have no files read yet\n");

  for(s=qp_sllist_begin(app->sources);s != last_s; s=qp_sllist_next(app->sources))
    offset += s->num_channels;

  for(j=0;j<len;++j)
  {
    x[j] += offset;
    y[j] += offset;

    if(x[j] >= s->num_channels || y[j] >= s->num_channels)
    {
      ERROR("can't plot channel %zu for --plot-file=\"%s\" option\n"
          "only %zu channels from file \"%s\"\n"
         "channel numbers start at 0\n",
         (x[j]>y[j])?x[j]:y[j], arg, s->num_channels, s->name);
      exit(1);
    }
  }

  if(qp_qp_graph(NULL, x, y, len, NULL))
    exit(1);
  
  parser->p2.needs_graph = 0;
}

static inline
void parse_2nd_point_size(char *arg, int argc, char **argv, int *i)
{
  if(!strncasecmp(arg, "AUTO", 4))
    app->op_point_size = -1;
  else
    app->op_point_size = get_long(arg, 1, 101, 3);
}

static inline
void parse_2nd_same_x_scale(char *arg, int argc, char **argv, int *i)
{
  if(arg)
    app->op_same_x_scale = get_yes_no_auto_int(arg, argv, i);
  else
    app->op_same_x_scale = 1;
}

static inline
void parse_2nd_same_y_scale(char *arg, int argc, char **argv, int *i)
{
  if(arg)
    app->op_same_y_scale = get_yes_no_auto_int(arg, argv,  i);
  else
    app->op_same_y_scale = 1;
}

static inline
void parse_2nd_skip_lines(char *arg, int argc, char **argv, int *i)
{
  app->op_point_size = get_long(arg, 0, INT_MAX - 10, 0);
}

