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



#ifdef DMALLOC
#  include "dmalloc.h"
#endif


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>


#include "config.h"
#include "debug.h"
#include "spew.h"
#include "list.h"

#ifdef QP_DEBUG
# include <signal.h>
#endif

#ifdef HAVE_LIBREADLINE
#  if defined(HAVE_READLINE_READLINE_H)
#    include <readline/readline.h>
#  elif defined(HAVE_READLINE_H)
#    include <readline.h>
#  else /* !defined(HAVE_READLINE_H) */
#    undef HAVE_LIBREADLINE
#  endif
#endif

#ifdef HAVE_READLINE_HISTORY
#  if defined(HAVE_READLINE_HISTORY_H)
#    include <readline/history.h>
#  elif defined(HAVE_HISTORY_H)
#    include <history.h>
#  else /* !defined(HAVE_HISTORY_H) */
#    undef HAVE_READLINE_HISTORY
#  endif
#endif /* HAVE_READLINE_HISTORY */


#ifdef DMALLOC
#  include "dmalloc.h"
#endif

static char *prompt = "QP> ";


#ifdef QP_DEBUG
static
void sighandler(int sig_num)
{
  VASSERT(0, "We caught signal %d", sig_num);
}
#endif

static inline
int Getline(char **line)
{
#ifdef HAVE_LIBREADLINE
  if(*line)
    free(*line);
  *line = readline(prompt);
  if(*line)
  {
#ifdef HAVE_READLINE_HISTORY
    if(**line)
      add_history(*line);
#endif
    return 1;
  }
  else
    return 0;
#else
  size_t len;

  printf("%s", prompt);
  fflush(stdout);

  if(getline(&line, &len, stdin) == -1)
  {
    if(*line)
      free(*line);
    return 0;
  }

  len = strlen(*line);
  ASSERT(len > 0);
  /* remove newline '\n' */
  (*line)[len-1] = '\0';
  return 1;
#endif
}

int main (int argc, char **argv)
{
  char *line = NULL;
  int ret;

#ifdef QP_DEBUG
  signal(SIGSEGV, sighandler);
  signal(SIGABRT, sighandler);
#endif

  if(isatty(fileno(stdout)))
  {
#ifdef HAVE_LIBREADLINE
    printf("Using readline version: %d.%d\n",
        RL_VERSION_MAJOR, RL_VERSION_MINOR);
#else
    printf("Using getline()\n");
#endif
  }

  while((ret = Getline(&line)))
  {
    printf("read %s\n", line);
  }

  printf("exiting\n");
  return 0;
}
