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

/* Quickplot is turning out to have so many command
 * line options that we are automating the coding and
 * documenting of them using this C code.
 * We will try to make this the only place with these
 * argument option strings so that we can change them
 * in one place.  Or better yet not change them.  
 *
 * One negative to this method is that when this file
 * is edited many other source files depend on
 * it so they have to be recompiled.  Reminds me of
 * using the QT moc preprocessor and why I do not like
 * QT.  The QT mess was much worse.  There was almost
 * no point in using make given any time you edited
 * a file all files would need recompiling.  Maybe
 * it just seemed that way. */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>

#include "config.h"
#include "debug.h"

struct qp_option
{
  /* 0 = do not call function    1 or 2 = call function    2 = function will exit */
  int pass[2]; /* pass[0] == 1st pass   pass[1] == 2nd pass */
  char *long_op; /* initialization code is generated based this string */
  char *short_op;
  char *arg; /* if arg == "[STRING]" then it is optional */
  char *description; /* you can put @CONFIG_STUFF@ in this string */
  /* Set these two to make code that initializes and declares a
   * struct qp_app variable. */
  char *op_init_value; /* like "0" for app->op_my_long_option = 0; */
  char *op_type;       /* like "int" for int op_my_long_option; in the struct  qp_app */
};


/*  description special sequences that make HTML markup
 *
 *    "**" = start list <ul> 
 *    "##"  = <li>
 *    "&&" = end list   </ul>
 *
 *    "  " = "&nbsp; "
 *
 *    "--*"  =  "<a href="#name">--*</a>"  where --* is a long_op
 *
 *    "::"   = "<span class=code>"  and will not add '\n' until @@
 *    "@@"   = "</span>"
 */


/* int tri values are   auto = -1   on or yes = 1    no or off = 0 */



static
struct qp_option options[] =
{
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {1,1}, "",                    0,           "FILE",      "read data from file FILE.  If FILE is - (dash) then "
                                                          "standard input will be read.  See also ::--file@@ and "
                                                          "::--pipe@@.",                                                0,       0          },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {2,0}, "--about",             "-a",        0,           "display introductory information about Quickplot in a "
                                                          "browser and exit",                                           0,       0          },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--auto-scale",        "-A",        0,           "automatically select if multi-plot graphs are on the "
                                                          "same x an y scales.  This is the default.  See also "
                                                          "::--same-x-scale@@, ::--same-y-scale@@, ::--same-scale@@ "
                                                          "::--different-scale@@.",                                     0,       0          },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {1,1}, "--background-color",  "-c",        "RGBA",      "set the color of the graph background.  RGBA may be any "
                                                          "string that GTK+ can parse into a RGB or RGBA color.  For "
                                                          "example ::--background-color='rgba(0,0,255,0.5)'@@ with make "
                                                          "translucent blue.",                                          0,       "struct "
                                                                                                                                 "qp_color"
                                                                                                                                 "a"        },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--border",            0,           0,           "add a border to main window.  This is the default.  "
                                                          "See also ::--no-border@@.",                                  "TRUE",  "gboolean" },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--buttons",           "-u",        0,           "show the button bar in the main window.  This is the "
                                                          "default.  See also ::--no-buttons@@.",                       "1",     "int"      },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--cairo-draw",        "-w",        0,           "draw graphs using the Cairo API.  Cairo drawing may be "
                                                          "slower, but you get translucent colors and anti-aliasing "
                                                          "in all aspects of the graph and in saved image files.  "
                                                          "See also ::--x11-draw@@.",                                   0,       0,         },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--default-graph",     "-D",        0,           "create the default graph for each file.  This is the "
                                                          "default.  If you give a ::--plot@@ or ::--plot-file@@ "
                                                          "after this options and before the next file no default "
                                                          "graph will be made.  A default graph will be made each "
                                                          "time this option is encountered.",                          "1",     "int"      },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--different-scales ", "-d",        0,           "graphs with more than one plot will have different "
                                                          "scales if the extreme values in each plot are not all "
                                                          "the same.  See also ::--same-scale@@.",                      0,       0          },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--file",              "-f",        "FILE",      "read data from file FILE.  If FILE is - (dash) then "
                                                          "standard input will be read.  See also ::--pipe@@.",         0,       0          },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--fullscreen",        0,           0,           "make the main window fullscreen.  See also "
                                                          "::--maximize@@.",                                            0,       0          },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--geometry",          0,           "GEO",       "specify the position and size of the main window.  To "
                                                          "set the geometry back to the default just set GEO "
                                                          "to NONE.  Example ::--geometry=1000x300-0+30@@",             "NULL",  "char *"   },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--grid",              0,           0,           "draw a grid with the graph.  This is the default.  "
                                                          "See also ::--no-grid@@.",                                    "1",     "int"      },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--grid-font",         0,           "FONT",      "set the font used to in the grid label numbers.  "
                                                          "Example: ::--grid-font='Sans Bold 12'@@.  The default "
                                                          "grid font is \""DEFAULT_GRID_FONT"\".",                       "qp_strdup(\""
                                                                                                                          DEFAULT_GRID_FONT
                                                                                                                          "\")",   "char *" },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--grid-line-width",   "-g",        "PIXELS",    "set the width of the grid lines if there are any",           "4",     "int"      },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {1,1}, "--grid-line-color",   "-C",        "RGBA",      "set the graph grid lines color.  RGBA may be any string "
                                                          "that GTK+ can parse into a RGB or RGBA color.  For "
                                                          "example ::--grid_line_color='rgba(255,0,0,0.5)'@@ with "
                                                          "make a translucent red.",                                    0,       "struct "
                                                                                                                                 "qp_color"
                                                                                                                                 "a"        },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--grid-numbers",      0,           0,           "show grid numbers.  This is the default.  The grid must "
                                                          "be showing to show "
                                                          "grid numbers too.  See also ::--no-grid-numbers@@.",         "1",     "int"      },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {1,1}, "--grid-text-color",   "-T",        "RGBA",      "set the graph grid text color.  RGBA may be any string "
                                                          "that GTK+ can parse into a RGB or RGBA color.  For "
                                                          "example ::--grid_text_color='rgba(0,255,0,0.5)'@@ with "
                                                          "make translucent green.",                                    0,       "struct "
                                                                                                                                 "qp_color"
                                                                                                                                 "a"        },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--grid-x-space",        0,        "PIXELS",     "set the maximum x space between vertical grid lines. "
                                                          "The minimum will be about half this.  This distance "
                                                          "varies as the scale changes due to zooming.  This "
                                                          "distance cannot be fixed due to the way Quickplot "
                                                          "scales your graphs and always picks reasonable grid "
                                                          "line spacing.  See also ::--grid-x-space@@.",                "220",   "int"      },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--grid-y-space",        0,        "PIXELS",     "set the maximum y space between horizontal grid lines.  "
                                                          "See also ::--grid-x-space@@ above.",                         "190",   "int"      },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {2,0}, "--gtk-version",       0,           0,           "print the version of GTK+ that Quickplot was built with "
                                                          "and then exit",                                              0,       0          },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--gui",               0,           0,           "show the menu bar, button bar, tabs bar, and the status "
                                                          "bar.  This is the default.  See also ::--no-gui@@.",         0,       0          },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {2,0}, "--help",              "-h",        0,           "display help in a browser and exit",                         0,       0          },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--label-separator",   "-b",        "STR",       "specifies the label separator string STR if labels are "
                                                          "read in from the top of a text data plot file.  The "
                                                          "default value of ::STR@@ is ::\" \"@@ (a single space).  "
                                                          "See option: ::--labels@@.",                                  "\" \"", "char *"   },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--labels",            "-L",        0,           "read labels from the top of a text data plot file.  See "
                                                          "option: ::--label-separator@@.",                             "0",     "int"      },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {2,0}, "--libsndfile-version",
                                0,           0,           "print the version of libsndfile that Quickplot was built "
                                                          "with and then exit",                                         0,       0          },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--line-width",        "-I",        "PIXELS",    "specify the plot line widths in pixels.  May be set to "
                                                          "AUTO to let Quickplot select the line with based on the "
                                                          "plot point density.  AUTO is the default.",                  "-1",    "int"      },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {1,1}, "--linear-channel",    "-l",        "[OPTS]",    "::OPTS@@ are ::[--start|-r VAL]@@ "
                                                          "::[--step|-t SIZE]@@.  "
                                                          "This option prepends a linear series channel to the file "
                                                          "being read "
                                                          "** ## ::--start VAL@@ or ::-r VAL@@  set the first "
                                                          "value in the sequence to VAL.  The default first value "
                                                          "is ::0@@. ## ::--step SIZE@@ or ::-t SIZE@@  set the "
                                                          "sequence step size to ::SIZE@@.  The default is 1. && "
                                                          "Sound files will always have a linear channel that "
                                                          "contains the time prepended.  "
                                                          "See also ::--no-linear-channel@@.",                          "NULL",  "struct "
                                                                                                                                 "qp_chann"
                                                                                                                                 "el *"     },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--lines",             "-H",        "[Y|N|A]",   " ** ## Y  yes show lines ## ::N@@  no don't show lines. "
                                                          "Same as ::--no-lines@@. ## ::A@@  auto, be smart about "
                                                          "it.  This is the default. && If the ::Y@@, ::N@@, or "
                                                          "::A@@ is not given ::Y@@ is implied.",                       "-1",    "int"      },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--maximize",          0,           0,           "maximize the main window.  See also ::--no-maximize@@.",     "0",     "int"
                                                                        /* app->op_maximize = 0 off     1 on    2 fullscreen */             },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--menubar",           0,           0,           "show the menu bar.  This is the default.  See also "
                                                          "::--menubar@@.",                                             "1",     "int"      },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--new-window",         0,           0,          "make a new main window for each graph",                      "0",     "int"      },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--no-border",         0,           0,           "display graphs main windows with no borders",                0,       0          },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--no-buttons",        "-B",        0,           "hide the button bar in the main window.  See also "
                                                          "::--buttons@@.",                                             0,       0          },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--no-fullscreen",     0,           0,           "don't make the main window fullscreen.  This is the "
                                                          "default.  See also ::--fullscreen@@.",                       0,       0          },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--no-default-graph",  "-U",        0,           "don't make the default graph for each file loaded.  "
                                                          "See also ::--default-graph@@.",                              0,       0          },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--no-grid",           "-G",        0,           "don't draw graph grid lines in the graph.  See "
                                                          "also ::--grid@@.",                                           0,       0          },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--no-grid-numbers",   0,           0,           "don't show grid numbers.  See also ::--grid-numbers@@.",     0,       0          },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--no-gui",            0,           0,           "don't show the menu bar, button bar, tabs bar, and "
                                                          "status bar.  See also ::--gui@@.",                           0,       0          },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--no-labels",         0,           0,           "don't read plot channel labels from the file.  This is "
                                                          "the deafult.  See also ::--labels@@.",                       0,       0          },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--no-linear-channel", 0,           0,           "turn off adding a linear channel for up coming files.  "
                                                          "See also ::--linear-channel@@.",                             0,       0          },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--no-lines",          "-i",        0,           "plot without drawing lines in the graph.  See also "
                                                          "--show-lines.",                                              0,       0          },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--no-maximize",       0,           0,           "don't maximize the main window.  This is the default.  "
                                                          "See also ::--fullscreen@@.",                                 0,       0          },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--no-menubar",        "-M",        0,           "don't display the menu bar in the main window.  See "
                                                          "also ::--menubar@@.",                                        0,       0          },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--no-new-window",      0,           0,          "don't make a new main window for the graph.  This is "
                                                          "the default.  See also ::--new-window@@.",                   0,       0          },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {1,0}, "--no-pipe",           "-N",        0,           "don't read data in from standard input even if there is "
                                                          "input to read.  See also ::--pipe@@.",                       0,       0          },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--no-points",         "-o",        0,           "plot without drawing points in the graph.  See also "
                                                          "--points.",                                                  0,       0          },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--no-shape",          0,           0,           "turn off the use of the X11 shape extension.  See "
                                                          "also ::--shape@@.",                                          0,       0          },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--no-statusbar",      0,           0,           "hide the status bar in the main window.  See "
                                                          "also ::--statusbar@@.",                                      0,       0          },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--no-tabs",           0,           0,           "don't show the graph tabs in the main window.  "
                                                          "See also ::--tabs@@.",                                       0,       0          },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--number-of-plots",   "-n",        "NUM",       "set the default maximum number of plots for each graph "
                                                          "to NUM",                                                     0,       "int"      },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {1,1}, "--pipe",              "-P",        0,           "read plot data from standard input.  By default "
                                                          "Quickplot looks for data from standard input and stops "
                                                          "looking if no data is found in some short amount of "
                                                          "time.  This option will cause Quickplot to wait for "
                                                          "standard input indefinitely.  If you would like to type "
                                                          "data in from the terminal use ::--pipe@@.  This option "
                                                          "is the same as ::--file=-@@.",                               "-1",    "int"      },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--plot",              "-p",        "LIST",      "plot the following list of channels, ::LIST@@.  Example: "
                                                          "::--plot \"0 1 3 4\"@@ will plot channel 1 VS channel 0 "
                                                          "and channel 4 VS channel 3 in the same graph.  Data "
                                                          "channels are numbered, starting at 0, in the order that "
                                                          "they are read in from all files or created, as in the "
                                                          "case of option ::--linear-channel@@.  A separate graph "
                                                          "tab will be created for each ::--plot@@ option given.  "
                                                          "This ::--plot@@ option must be after the file loading "
                                                          "options that load the channels that it lists to plot.  "
                                                          "See also ::--plot-file@@.",                                  0,       0          },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--plot-file",         "-F",        "LIST",      "plot the following list of channels, ::LIST@@.  "
                                                          "Example: ::--plot-file \"0 1 3 4\"@@ will plot channel 1 "
                                                          "VS "
                                                          "channel 0 and channel 4 VS channel 3 in the same graph.  "
                                                          "This is like the ::--plot@@ option except that this will "
                                                          "only make a graph for the last file preceding this "
                                                          "option and the channel numbers are for the channels just "
                                                          "from that last file.  They are relative channel numbers.  "
                                                          "This is handy if you load a lots of files and lose "
                                                          "count of the number of channels loaded in each file.",       0,       0          },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--point-size",        "-O",        "PIXELS",    "start Quickplot using plot point size ::PIXELS@@ "
                                                          "wide in pixels.  "
                                                          "This may be set to ::AUTO@@ to have quickplot "
                                                          "automatically set the point size depending on the point "
                                                          "density that is in graph.  ::AUTO@@ is the default.",       "-1",     "int"      },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--points",            0,           0,           "show points in the plots in the graph.  This is the "
                                                          "default.",                                                   "1",     "int"      },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {2,0}, "--print-about",       0,           0,           "prints the About document to standard output andthen "
                                                          "exits.  Use option ::--about@@ to display an HTML version "
                                                          "of the Quickplot About information.",                        0,       0          },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {2,0}, "--print-help",        0,           0,           "prints this Help document as ASCII text to standard "
                                                          "output and then exits.  Use option ::--help@@ for "
                                                          "displaying an HTML version of this help.",                   0,       0          },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {1,1}, "--read-pipe-here",    "-R",        0,           "this is a place holder that tells Quickplot when to read "
                                                          "the data from standard input.  This is intended to give "
                                                          "the option of telling Quickplot when to read standard "
                                                          "input when Quickplot automatically determines whether to "
                                                          "read standard input or not.  See options ::--file@@, "
                                                          "::--pipe@@ and ::--no-pipe@@.",                              "0",     "int"      },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--same-scale",        "-s",        0,           "plot all in a graph plots on the same scale.  See also "
                                                          "::--different-scale@@, ::--same-x-scale@@ and "
                                                          "::--same-y-scale@@.",                                        0,       0          },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--same-x-scale",      "-x",        "[Y|N|A]",   "use in place of ::--same-scale@@ or ::--auto-scale@@ for "
                                                          "finer "
                                                          "control over how the x values of the plots are scaled "
                                                          "when you have more than one plot on a graph ** ## "
                                                          "::Y@@  yes same x scale ## ::N@@  no different x "
                                                          "scales ## ::A@@  auto, "
                                                          "be smart about it && If the ::Y@@, ::N@@, or ::A@@ "
                                                          "is not given ::Y@@ is implied.",                           "-1",    "int"      },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--same-y-scale",      "-y",        "[Y|N|A]",   "use in place of ::--same-scale@@ or ::--auto-scale@@ for "
                                                          "finer control over how the x values of the plots are "
                                                          "scaled when you have more than one plot on a graph ** "
                                                          "## ::Y@@  yes "
                                                          "same y scale ## ::N@@  no different y scales ## "
                                                          "::A@@  auto, be smart about it.  This is the default. && If "
                                                          "the ::Y@@, "
                                                          "::N@@, or ::A@@ is not given Y is implied.",           "-1",    "int"      },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--shape",             0,           0,           "make graphs see through.  This is insanely cool.  It "
                                                          "uses the X11 shape extension which was made famous by "
                                                          "xeyes.  The X11 shape extension may be a little flashy "
                                                          "on some systems.  Try using "
                                                          "::--shape@@ with the ::--no-gui@@, ::--no-grid@@, and "
                                                          "::--no-border@@ options to make a floating plot line "
                                                          "on your display.  The use of the X11 shape extension "
                                                          "is a property of the main window, not each graph tab.  "
                                                          "This option may not work well with fullscreen view.  "
                                                          "This is will slow down graph drawing considerably.  "
                                                          "You can toggle this on and off with the ::x@@ key.  "
                                                          "See option ::--no-shape@@.",                                 "0",     "int"      },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {1,0}, "--silent",            0,           0,           "don't spew even on error.  The ::--silent@@ option will "
                                                          "override the effect of the ::--verbose@@ option.",           0,       0          },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--skip-lines",        "-S",        "NUM",       "skip the first ::NUM@@ lines when reading the file.  This "
                                                          "just applies when reading text files.",                      "0",     "size_t"   },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--statusbar",         0,           0,           "show the status bar below the graph.  This is the "
                                                          "default.  See also ::--no-statusbar@@",                      "1",     "int"      },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--tabs",              0,           0,           "show the graph tabs.  This is the default.  See also "
                                                          "::--no-tabs@@.",                                             "1",     "int"      },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {1,0}, "--verbose",           "-v",        0,           "spew more to standard output.  See also ::--silent@@.",      0,       0          },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {2,0}, "--version",           "-V",        0,           "print the Quickplot version number and then exit "
                                                          "returning 0 exit status",                                    0,       0          },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,1}, "--x11-draw",          "-X",        0,           "draw points and lines using the X11 API.  This is the "
                                                          "default.  Drawing may be much faster than with "
                                                          "cairo, but there will be no translucent colors and "
                                                          "anti-aliasing in the drawing of the plot lines and "
                                                          "points. "
                                                          "There will be translucent colors and anti-aliasing in "
                                                          "the background and grid, but not in saved images.  "
                                                          "You can start drawing with X11 and switch to drawing with "
                                                          "Cairo when you want to save an image.  That will save you "
                                                          "time.  See also ::--cairo-draw@@.",                          "1",     "int"      },
/*-----------------------------------------------------------------------------------------------------------------------------------------*/
{ {0,0}, 0,                     0,           0,           0,                                                            0,       0          }
};



/* the length of the array of options */
static size_t len = 0;
/* the sorted array of options */
static struct qp_option **opt;



static inline
void *Malloc(size_t s)
{
  void *p;
  errno = 0;
  p = malloc(s);
  if(!p)
  {
    printf("malloc(%zu)\n", s);
    perror("malloc()");
    while(1) sleep(100);
  }
  return p;
}

static const char *prefix[2] = { "parse_1st_", "parse_2nd_" };
static char *func = NULL;


static inline
char *get_func(const char *prefix, const char *str)
{
  char *s;
  if(!str || !str[0])
    str = "File";

  if(func)
    free(func);
  s = (char *) str;
  while(*s == '-') ++s;

  func = Malloc(strlen(prefix) + strlen(s) + 1);
 
  sprintf(func, "%s%s", prefix, s);

  s = func;
  while(*s)
  {
    if(*s == '-')
      *s = '_';
    ++s;
  }

  return func;
}

static
int not_short_op_switch_case(struct qp_option *op, int pass)
{
  if(pass == 1 && op->pass[0] == 2)
    /* We do not need to parse options that would
     * exit in the 1st pass */
    return 1;

  return (
              (op->arg && op->arg[0] != '[')
          || !op->short_op
          || (strlen(op->short_op) != 2)
      );
}

static
void print_arg_parsing_code(int p)
{
  size_t i;
  int got_one = 0;
  int got_two = 0;


  printf(
      "{\n"
      "   int i = 1;\n"
      "   char *s;\n"
      "\n"
      "   while(i < argc)\n"
      "   {\n"
  );

  for(i=1;i<len;++i)
  {
    if(!opt[i]->pass[p] || opt[i]->arg)
      continue;

    func = get_func(prefix[p], opt[i]->long_op);

    printf("     %sif(!strcmp(\"%s\", argv[i]))\n"
           "     {\n"
           "        %s();\n%s"
           "     }\n",
           (got_one)?"else ":"",
           opt[i]->long_op, func,
           (opt[i]->pass[p] == 1)?"        ++i;\n":"");
    ++got_one;
  }


  for(i=1;i<len;++i)
  {
    if(!opt[i]->arg)
      continue;

    func = get_func(prefix[p], opt[i]->long_op);

    printf(
           "     %sif((s = get_opt(",(got_one)?"else ":"");
    if(opt[i]->short_op)
      printf("\"%s\"", opt[i]->short_op);
    else
      printf("NULL");
    printf(",\"%s\", argc, argv, &i)))\n"
           "     {\n", opt[i]->long_op);

    if(opt[i]->pass[p])
      printf(
           "        %s(s, argc, argv, &i);\n", func);
    else
      printf(
           "        /* do nothing this pass */\n");

    printf(
           "     }\n");
    
    if(opt[i]->arg[0] == '[')
    {  /* again for case with no optional argument */
      printf(
          "     %sif(!strcmp(\"%s\",argv[i])",
          (got_one)?"else ":"", opt[i]->long_op);
      if(opt[i]->short_op)
        printf(" || !strcmp(\"%s\",argv[i])",
            opt[i]->short_op);
      printf(
          ")\n"
          "     {\n"
          "        /* this is to caught it when there is no optional arg */\n"
          );
      if(opt[i]->pass[p])
        printf(
          "        %s(NULL, argc, argv, &i);\n", func);
      else
        printf(
          "        /* do nothing this pass */\n"
          );
      printf(
          "        ++i;\n"
          "     }\n");
    }

    ++got_one;
  }

  
  for(i=1;i<len;++i)
  {
    if(!opt[i]->arg && !opt[i]->pass[p] &&
        (p == 0 || opt[i]->pass[0] != 2))
    {
      printf("     %sif(!strcmp(\"%s\", argv[i])\n",
             (got_one)?"else ":"",
             opt[i]->long_op);
      ++got_two;
      break;
    }
  }
  
  for(++i;i<len;++i)
  {
    if(!opt[i]->arg && !opt[i]->pass[p] &&
        (p == 0 || opt[i]->pass[0] != 2))
    {
      printf("          || !strcmp(\"%s\", argv[i])\n",
             opt[i]->long_op);
      continue;
    }
  }

  if(got_two)
    printf(  "          )\n"
             "      {\n"
             "         /* do nothing this pass */\n"
             "         ++i;\n"
             "      }\n");




  got_two = 0;


  for(i=1;i<len;++i)
  {
    if(not_short_op_switch_case(opt[i], p))
        continue;

    printf("     %sif(argv[i][0] == '-' && \n"
           "         (   argv[i][1] == '%c'\n",
         (got_one)?"else ":"",
         opt[i]->short_op[1]);
    ++got_two;
    break;
  }
  
  for(++i;i<len;++i)
  {
    if(not_short_op_switch_case(opt[i], p))
      continue;

    printf("          || argv[i][1] == '%c'\n",
          opt[i]->short_op[1]);
  }

  if(got_two)
  {
    printf("       )\n"
        "        )\n"
        "    {\n"
        "      switch(argv[i][1])\n"
        "      {\n"
        );



    for(i=1;i<len;++i)
    {
      if(not_short_op_switch_case(opt[i], p) || !opt[i]->pass[p])
        continue;

        printf(  "        case '%c':\n", opt[i]->short_op[1]);
        func = get_func(prefix[p], opt[i]->long_op);
        if(!opt[i]->arg)
          printf("          %s();\n", func);
        else
          printf("          %s(NULL, argc, argv, &i);\n", func);
        printf( "          ++i;\n"
              "          break;\n");
    }


    /* reuse dummy flag */
    got_two = 0;

    for(i=1;i<len;++i)
    {
      if(not_short_op_switch_case(opt[i], p)  || opt[i]->pass[p])
        continue;

      printf(  "        case '%c':\n", opt[i]->short_op[1]);
      got_two = 1;
    }

    if(got_two)
      printf( "          ++i;\n"
              "          /* do nothing this pass */\n"
              "          break;\n");

    printf("        }\n"
           "      }\n"
        );
  }


  printf("    else\n"
         "    {\n"
         "      /* option FILE */\n");
  
  if(opt[0]->pass[p])
    printf("      %s(argv[i]);\n", get_func(prefix[p], opt[0]->long_op));
  else
    printf("      /* do nothing this pass */\n");
  printf(  "      ++i;\n");


  printf(
         "    }\n"
         "  }\n"
         "\n"
         "}\n"
         "\n");
}

static
void print_argument_parsing_code(void)
{
  printf("\nstatic\n"
      "void parse_args_1st_pass(int argc, char **argv)\n");

  print_arg_parsing_code(0);

  printf("\nstatic\n"
      "void parse_args_2nd_pass(int argc, char **argv)\n");

  print_arg_parsing_code(1);
}

static
void print_pass_functions(int p)
{
  int i;

  printf("\n");

  /* FILE is a argument with an long option flag */
  if(opt[0]->pass[p])
    printf("static inline\n"
        "void %s(const char *file)\n"
        "{\n"
        "\n"
        "}\n"
        "\n",
        get_func(prefix[p], opt[0]->long_op));

  for(i=1;i<len;++i)
  {
    if(!opt[i]->pass[p] || opt[i]->arg)
      continue;

    func = get_func(prefix[p], opt[i]->long_op);
    printf("static inline\n"
        "void %s(void)\n"
        "{\n"
        "\n"
        "}\n"
        "\n", func);
  }

  printf("\n");

  for(i=1;i<len;++i)
  {
    if(!opt[i]->pass[p] || !opt[i]->arg)
      continue;

    func = get_func(prefix[p], opt[i]->long_op);
    printf("static inline\n"
        "void %s(char *arg, int argc, char **argv, int *i)\n"
        "{\n"
        "\n"
        "}\n"
        "\n", func);
  }

  printf("\n");
}

static
void print_1st_pass_functions(void)
{
  print_pass_functions(0);
}

static
void print_2nd_pass_functions(void)
{
  print_pass_functions(1);
}

static
void print_app_ops_declare_code()
{
  int i;

  for(i=1;i<len;++i)
  {
    if(!opt[i]->op_type)
      continue;

    printf("%s %s;\n", opt[i]->op_type,
        get_func("op_", opt[i]->long_op));
  }
}

static
void print_app_ops_init_code()
{
  int i;

  for(i=1;i<len;++i)
  {
    if(!opt[i]->op_init_value)
      continue;

    printf("  app->%s = %s;\n",
        get_func("op_", opt[i]->long_op),
        opt[i]->op_init_value);
  }
}

static
int get_string_len(char *str)
{
  if(!str)
    return 1;

  {
    size_t len;
    char *s;
    len = strlen(str);
    for(s=str;*s;++s)
      if(*s == '"')
        ++len;

    return len;
  }
}

/* adds space to make the size you print be n */
/* returns the number of chars printed not including '\0' */
static
int Nprintf(int n, const char *format, ...)
{
  va_list ap;
  char *fmt, *str;
  int i, j, len;

  j = strlen(format);
  len = n + j + 1;
  fmt = Malloc(len + 1);
  str = Malloc(len + 1);

  strcpy(fmt, format);

  for(i=j;i<len;++i)
    fmt[i] = ' ';
  fmt[i] = '\0';

  va_start(ap, format);
  vsnprintf(str, n+1, fmt, ap);
  va_end(ap);
  printf("%s", str);

  len = strlen(str);

  free(fmt);
  free(str);

  return len;
}

static
char *fix_quotes(char *str)
{
  size_t len, i;
  char *s, *ret;

  if(!str)
    return str;


  len = strlen(str) + 1;
  for(s=str;*s;++s)
    if(*s == '"')
      ++len;

  ret = (s = Malloc(len));

  --len;
  for(i=0;i<len;++i)
  {
    if(str[i] == '"')
    {
      *s = '\\';
      ++s;
      *s = '"';
    }
    else
      *s = str[i];
    ++s;
  }
  *s = '\0';

  return ret;
}
  

/* a snprintf() wrapper that I can get my head around. */
static inline
int Snprintf(char *str, size_t size, const char *format, ...)
{
  va_list ap;
  int ret;
  va_start(ap, format);
  vsnprintf(str, size+1, format, ap);
  va_end(ap);

  ret = strlen(str);

  return ret;
}

/* the str="0"  or str="hi aldnfa sdfkl" and so on  */
static
void kick_it_print(char *str, int start, int len, int add_comma)
{
  int slen;
  char *comma;

  if(add_comma)
    add_comma = 1;

  if(add_comma)
    comma = ",";
  else
    comma = "";

  if(!str)
  {
    Nprintf(len, "0%s", comma);
    return;
  }

  slen = strlen(str);


  while(1)
  {
    int i;

    if(slen <= len - (2 + add_comma))
    {
      Nprintf(len, "\"%s\"%s", str, comma);
      return;
    }

    for(i = len - (2 + add_comma); i> len/2 ; --i)
      if(str[i-1] == ' ')
        break;
    if(i == len/2)
      i = len - (2 + add_comma);

    printf("\"");
    slen -= i;
    Nprintf(i, "%s", str);
    str += i;
    printf("\"\n");


    Nprintf(start, " ");
    if(!add_comma)
      printf("  ");
  }
}    


static
void print_tidy_struct(void)
{
  int i;
  int count[7] = {0,0,0,0,0,0,0};

  printf("static\n"
      "struct qp_option options[] =\n"
      "{\n");

  for(i=0;i<len;++i)
  {
    int len;

    len = get_string_len(opt[i]->long_op) + 4;
    if(len > count[1])
      count[1] = len;
    
    len = get_string_len(opt[i]->short_op) + 4;
    if(len > count[2])
      count[2] = len;

    len = get_string_len(opt[i]->arg) + 4;
    if(len > count[3])
      count[3] = len;

    len = get_string_len(opt[i]->op_init_value) + 4;
    if(len > count[5])
      count[5] = len;

    len = get_string_len(opt[i]->op_type) + 3;
    if(len > count[6])
      count[6] = len;
  }


  for(i=1;i<7;i++)
    if(count[i] > 26)
      count[i] = 26;

  if(count[3] > 13)
    count[3] = 13;
  if(count[5] > 10)
    count[5] = 10;
  if(count[6] > 10)
    count[6] = 10;

  
  count[0] = strlen("{ {1,1}, ");
  count[4] = 60;

  /* change the counts to absolute distance */
  for(i=1;i<7;i++)
    count[i] += count[i-1];


  printf("/*");
  for(i=0;i<count[6];++i)
    putchar('-');
  printf("*/\n");


  for(i=0;i<len;++i)
  { 
    char *s[6];
    int j;
    s[0] = fix_quotes(opt[i]->long_op);
    s[1] = fix_quotes(opt[i]->short_op);
    s[2] = fix_quotes(opt[i]->arg);
    s[3] = fix_quotes(opt[i]->description);
    s[4] = fix_quotes(opt[i]->op_init_value);
    s[5] = fix_quotes(opt[i]->op_type);
    Nprintf(count[0], "{ {%d,%d},", opt[i]->pass[0], opt[i]->pass[1]);
    for(j=0;j<6;++j)
    {
      kick_it_print(s[j], count[j], count[j+1] - count[j], (j!=5)?1:0);
      if(j==3)
        printf("  ");
      if(s[j])
        free(s[j]);
    }
    printf(" },\n/*");
    for(j=0;j<count[6];++j)
      putchar('-');
    printf("*/\n");
  }

  Nprintf(count[0], "{ {0,0},");
  for(i=0;i<6;++i)
  {
    kick_it_print(NULL, count[i], count[i+1] - count[i], (i!=5)?1:0);
    if(i==3)
      printf("  ");
  }
  printf(" }\n");

  printf("};\n");
}

static
void print_help_text(void)
{

}

static
void print_html_options_list(void)
{

}

/* returns length printed
 * adds <a href=#LONG_OPT_NAME> 
 * if the str have a --long-opt in it.
 * Or returns 0 is none is found. */
int check_print_long_option_link(const char *str, size_t skip_i)
{
  size_t i;
  for(i=1;i<len;++i)
  {
    size_t len_opt, len_str;
    if(i == skip_i)
      continue;

    len_str = strlen(str);
    len_opt = strlen(opt[i]->long_op);
    if(!strncmp(opt[i]->long_op, str, len_opt) && len_str >= len_opt + 2 &&
        (
         (str[len_opt] == '@' && str[len_opt+1] == '@') ||
          str[len_opt] == ' ' || str[len_opt] == '='
        )
      )
      return printf("<a class=opt href=\"#%s\">", get_func("op_", opt[i]->long_op));
  }
  return 0;
}

static
void print_text2html(const char *str, size_t opt_i)
{
  char *s, *t;
  int w_count = 0;

  if(!str || !(*str) || !*(str +1))
  {
    printf("%s\n", str);
    return;
  }

  s = (char *) str;
  t = s + 1;
  while(*s && *t)
  {
    if(*s == '*' && *t == '*')
    {
      if(w_count)
        putchar('\n');
      printf("      <ul>\n");
      s += 2;
      while(*s == ' ')
        ++s;
      t = s + 1;
      w_count = 0;
      continue;
    }
    if(*s == '#' && *t == '#')
    {
      if(w_count)
        putchar('\n');
      printf("      <li>");
      s += 2;
      while(*s == ' ')
        ++s;
      t = s + 1;
      w_count = 10;
      continue;
    }
    if(*s == ' ' && *t == ' ')
    {
      w_count += printf("&nbsp; ");
      s += 2;
      t += 2;
      continue;
    }
    if(*s == ':' && *t == ':')
    {
      int do_ancher;

      if(w_count)
      {
        putchar('\n');
        w_count = 0;
      }
      w_count += printf("      <span class=code>");
      s += 2;
      t += 2;

      w_count += (do_ancher = check_print_long_option_link(s, opt_i));

      while(*s && !(*s == '@' && *t == '@'))
      {
        putchar(*(s++));
        t = s + 1;
        ++w_count;
      }
      if(do_ancher)
        w_count += printf("</a>");
      w_count += printf("</span>");
      s += 2;
      if(*s == ' ' && *(s+1) != ' ')
      {
        putchar('\n');
        w_count = 0;
        ++s;
      }
      t = s + 1;
      continue;
    }
    if(*s == '&' && *t == '&')
    {
      if(w_count)
        putchar('\n');
      printf("      </ul>\n");
      s += 2;
      while(*s == ' ')
        ++s;
      t = s + 1;
      w_count = 0;
      continue;
    }

    if(w_count > 50 && *s == ' ')
    {
      putchar('\n');
      w_count = 0;
    }
    else
    {
      ++w_count;
      putchar(*s);
    }

    ++t;
    ++s;
  }
  if(*s)
    putchar(*s);
  putchar('\n');
}

static
void print_html_options_table(void)
{
  int i;
  char class[2] = { 'a', 'b' };

  printf(
    "<table class=options summary=\"Quickplot argument options\">\n"
    );


  printf(
    "  <tr>\n"
    "    <th class=opt>long options</th>\n"
    "    <th class=opt title=\"short options\" style=\"%s\">short</th>\n"
    "    <th class=opt title=\"option arguments\">arg</th>\n"
    "    <th class=opt>description</th>\n"
    "  </tr>\n"
    "\n", "font-size:50%;"
    );

  for(i=0;i<len;++i)
  {
    printf(
    "  <tr class=%c>\n",
    class[i%2]);

    printf(
    "    <td class=opt style=\"white-space:nowarp;\">\n"
    "      <a name=\"%s%s\">%s</a>\n"
    "    </td>\n", i?"":"FILE",
    get_func("op_", opt[i]->long_op), opt[i]->long_op);

    printf(
    "    <td class=opt style=\"white-space:nowarp;\">\n"
    "      %s\n"
    "    </td>\n",
    opt[i]->short_op?opt[i]->short_op:"");

    printf(
    "    <td class=opt>\n"
    "      %s\n"
    "    </td>\n",
    opt[i]->arg?opt[i]->arg:"");
    
    printf(
    "    <td class=opt>\n");
    print_text2html(opt[i]->description, i);
    printf(
    "    </td>\n");

        printf(
    "  </tr>\n");
  }

  printf("</table>\n");


}

int main(int argc, char **argv)
{
  /* options: argc == 1 or argv[1] = "whatever" : help
   *          argv[1] = "-t"  print html options table
   *          argv[1] = "-l"  print html options list
   *          argv[1] = "-h"  print C code for OPTIONS help text
   *          argv[1] = "-a"  print argument parsing C code
   *          argv[1] = "-1"  print 1st pass parsing C code template
   *          argv[1] = "-2"  print 2nd pass parsing C code template
   *          argv[1] = "-i"  print C code for app->op_ initialization
   *          argv[1] = "-I"  print C code for declaring app
   *          argv[1] = "-T"  print C code of the big struct "Tidy"
   */
  {
    ssize_t n = 2;
    if(argc > 1)
      n = strlen(argv[1]);

    if(argc < 2 || argc > 3 || n != 2 ||
          argv[1][0] != '-' ||
          (argv[1][1] != 't' && argv[1][1] != 'l' && argv[1][1] != 'h' &&
           argv[1][1] != 'a' && argv[1][1] != '1' && argv[1][1] != '2' &&
           argv[1][1] != 'i' && argv[1][1] != 'I' && argv[1][1] != 'T')
          )
    {
      printf("Usage: %s [ -a | -h | -i | -I | -l | -t | -1 | -2 ]\n"
          "\n"
          "  Generate HTML and C code that is"
          " related to Quickplot command line options\n"
          "  Returns 0 on success and 1 on error.\n"
          "\n"
          "    -a  print C code for argument parsing\n"
          "    -h  print C code for OPTIONS help text\n"
          "    -i  print C code for app->op_ initialization\n"
          "    -I  print C code for declaring app\n"
          "    -l  print html options list\n"
          "    -t  print html options table\n"
          "    -T  print C code of the big qp_options array\n"
          "    -1  print 1st pass parsing C code template\n"
          "    -2  print 2nd pass parsing C code template\n"
          "\n", argv[0]);
      return 1;
    }
  }

  {
    int err = 0;
    /* make an array that we can sort */
    struct qp_option *o;
    /* Test that the options do not have duplicates */
    for(o=(struct qp_option *)options;o->long_op;++o)
    {
      struct qp_option *oc;
      for(oc=(o+1);oc->long_op;++oc)
      {
        if(!strcmp(oc->long_op, o->long_op))
        {
          printf("There are at least two long options:  %s\n", o->long_op);
          err = 1;
        }
        if(oc->short_op && o->short_op && !strcmp(oc->short_op, o->short_op))
        {
          printf("There are at least two short options:  %s\n", o->short_op);
          printf("one for: %s\n", o->long_op);
          printf("one for: %s\n", oc->long_op);
          err = 1;
        }
      }
      ++len;
    }
    if(err)
      return 1;
  }

  {
    /* sort the array */
    ssize_t i, end;

    opt = Malloc(sizeof(*opt)*len);
    /* sort the options in order of long option */
    for(i=0;i<len;++i)
      opt[i] = (struct qp_option *) &options[i];

    /* bubble sort for this small list */
    end = len - 2;
    while(end >= 0)
    {
      for(i=0;i<end;++i)
      {
        if(strcmp(opt[i]->long_op, opt[i+1]->long_op) > 0)
        {
          struct qp_option *o;
          o = opt[i];
          opt[i] = opt[i+1];
          opt[i+1] = o;
        }
      }
      --end;
    }
  }

#if 0
  if(argv[1][1] == 'a')
  {
    size_t i;

    printf("---- OLD SORT -----\n");
    for(i=0;i<len;++i)
      printf("%s\n", options[i].long_op);

    printf("---- NEW SORT ----\n");
    for(i=0;i<len;++i)
      printf("%s\n", opt[i]->long_op);
  }
#endif

  if(opt[0]->long_op[0])
  {
    printf("FILE is not the first option listed\n");
    exit(1);
  }

  if(argv[1][1] == 'T')
  {
    print_tidy_struct();
    exit(0);
  }

  if(argv[1][1] == 'I')
  {
    printf("/* This file was auto-generated by running: `%s %s' */\n",
        argv[0], argv[1]);
    print_app_ops_declare_code();
    exit(0);
  }

  if(argv[1][1] == 'i')
  {
    printf("/* This file was auto-generated by running: `%s %s' */\n",
        argv[0], argv[1]);
    print_app_ops_init_code();
    exit(0);
  }


  if(argv[1][1] == 'a')
  {
    printf("/* This file was auto-generated by running: `%s %s' */\n",
        argv[0], argv[1]);
    print_argument_parsing_code();
    exit(0);
  }

  if(argv[1][1] == 'h')
  {
    print_help_text();
    exit(0);
  }

  if(argv[1][1] == 'l')
  {
    print_html_options_list();
    exit(0);
  }

  if(argv[1][1] == 't')
  {
    printf("<!-- This table was auto-generated by running: `%s %s' -->\n",
        argv[0], argv[1]);
    print_html_options_table();
    exit(0);
  }

  if(argv[1][1] == '1')
  {
    printf("/* This file was auto-generated by running: `%s %s' */\n",
        argv[0], argv[1]);
    print_1st_pass_functions();
    exit(0);
  }

  if(argv[1][1] == '2')
  {
    printf("/* This file was auto-generated by running: `%s %s' */\n",
        argv[0], argv[1]);
    print_2nd_pass_functions();
    exit(0);
  }

  return 0;
}
