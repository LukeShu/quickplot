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
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>

#include "config.h"
#include "debug.h"
#include "spew.h"
#include "list.h"
#include "shell_common.h"

#ifdef DMALLOC
#  include "dmalloc.h"
#endif

static char *prompt = "QP> ";

#ifdef HAVE_LIBREADLINE
int use_readline = 0;
#endif


#ifdef QP_DEBUG
static
void debug_sighandler(int sig_num)
{
  VASSERT(0, "We caught signal %d", sig_num);
}
#endif

static
void term_sighandler(int sig_num)
{
  printf(
      "We caught signal %d\n"
      "enter \"exit\" to exit\n", sig_num);
}

static inline
int Getline(char **line, size_t *len)
{
#ifdef HAVE_LIBREADLINE
  if(use_readline)
  {
    if(*line)
      free(*line);
    *line = readline(prompt);
    if(*line)
    {
#   ifdef HAVE_READLINE_HISTORY
      if(**line)
        add_history(*line);
#   endif
      return 1;
    }
    else
      return 0;
  }
  else
  {
#endif
    size_t l;

    printf("%s", prompt);
    fflush(stdout);

    if(getline(line, len, stdin) == -1)
    {
      return 0;
    }

    l = strlen(*line);
    ASSERT(l > 0);
    /* remove newline '\n' */
    (*line)[l-1] = '\0';
    return 1;

#ifdef HAVE_LIBREADLINE
  }
#endif
}

void usage(void)
{
  printf(
      "  Usage: quickplot_shell PID\n"
      "\n"
      "  [This is not funtional yet.  It just runs like a client to an echo server.]\n"
      "  Connect to a running Quickplot program with process ID (pid) PID.\n"
      "  This will setup two named pipes /tmp/quickplot_to_PID_num and\n"
      "  /tmp/quickplot_from_PID_num, signal the running Quickplot program\n"
      "  with pid PID, and write commands and read responses to and from the\n"
      "  pipe.  The named pipes will be removed when this program cleanly exits.\n");
  exit(1);
}

/* returns 1 on error
 * returns 0 on success */
static inline
int read_and_spew_until_end(char **reply, size_t *len, FILE *from, pid_t pid)
{
  do
  {
    if(getline(reply, len, from) == -1)
    {
      printf("Quickplot pid %d: broke the connection.\n", pid);
      return 1;
    }
    if(!strcmp(*reply, "END\n"))
    {
      DEBUG("END\n");
      return 0;
    }
    printf(": %s", *reply);
  }
  while(!feof(from) ||
      check_file_in(from, 10 /* seconds */, 500000 /* micro seconds */));

  EWARN("failed to recieve END reply after 10.5 seconds\n");

  return 0;
}

int main (int argc, char **argv)
{
  char *user_line = NULL, *qp_reply = NULL;
  size_t qp_reply_len = 0, user_len = 0;
  int ret = 0;
  pid_t pid;
  /* reading to the from Quickplot */
  FILE *file_to = NULL, *file_from = NULL;
  char path_to[FIFO_PATH_LEN], path_from[FIFO_PATH_LEN];
  *path_to = '\0';
  *path_from = '\0';


  if(argc != 2 || argv[1][0] > '9' || argv[1][0] < '0')
    usage();

  {
    char *end;
    pid = strtoul(argv[1], &end, 10);
    if(*end != '\0')
      usage();
  }

  errno = 0;
  if(kill(pid, SIGCONT))
  {
    QP_EERROR("Failed to signal pid %d\n", pid);
    return 1;
  }

  errno = 0;
  if(mkfifo(set_to_qp_fifo_path(path_to, pid, getpid()),
        S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP))
  {
    QP_EERROR("Failed to create fifo %s\n", path_to);
    *path_to = '\0';
    ret = 1;
    goto cleanup;
  }
  errno = 0;
  if(mkfifo(set_from_qp_fifo_path(path_from, pid, getpid()),
        S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP))
  {
    QP_EERROR("Failed to create fifo %s\n", path_from);
    *path_from = '\0';
    ret = 1;
    goto cleanup;
  }
  /* Since the FIFO block on fopen(,"r") we will signal
   * before we call fopen(,"r").
   *
   * Looks like fopen(,"w") blocks too. */
  errno = 0;
  if(kill(pid, SIGUSR1))
  {
    QP_EERROR("Failed to signal pid %d\n", pid);
    return 1;
  }

  errno = 0;
  if(!(file_to = fopen(path_to, "w")))
  {
    QP_EERROR("fopen(\"%s\",\"a\") failed\n", path_to);
    ret = 1;
    goto cleanup_with_signal;
  }
  errno = 0;
  if(!(file_from = fopen(path_from, "r")))
  {
    QP_EERROR("fopen(\"%s\",\"r\") failed\n", path_from);
    ret = 1;
    goto cleanup_with_signal;
  }
  setlinebuf(file_from);


#ifdef QP_DEBUG
  signal(SIGSEGV, debug_sighandler);
  signal(SIGABRT, debug_sighandler);
#endif

  signal(SIGTERM, term_sighandler);
  signal(SIGQUIT, term_sighandler);
  signal(SIGINT, term_sighandler);

#ifdef HAVE_LIBREADLINE
  if(isatty(fileno(stdout)))
  {
    use_readline = 1;
    printf("Using readline version: %d.%d\n",
        RL_VERSION_MAJOR, RL_VERSION_MINOR);
  }
  else
  {
#else
    printf("Using getline()\n");
#endif
#ifdef HAVE_LIBREADLINE
  }
#endif

  /* Exercise the pipe so it does not block the Quickplot
   * service on fopen(,"r"). */
  fprintf(file_to, "START\n");
  fflush(file_to);

  if(read_and_spew_until_end(&qp_reply, &qp_reply_len, file_from, pid))
    goto cleanup_with_signal;

  /* We can hide the FIFO files now that both parties are connected.
   * The OS will keep the FIFO files in existence so long as one of
   * the two processes have them open. */
  unlink(path_to);
  *path_to = '\0';
  unlink(path_from);
  *path_from = '\0';

  while((Getline(&user_line, &user_len)))
  {
    printf("Read user line \"%s\"\n", user_line);
    fwrite(user_line, strlen(user_line), 1, file_to);
    putc('\n', file_to);
    fflush(file_to);

    if(read_and_spew_until_end(&qp_reply, &qp_reply_len, file_from, pid))
      goto cleanup_with_signal;
  }

cleanup_with_signal:

  errno = 0;
  if(kill(pid, SIGUSR2))
    QP_EERROR("Failed to signal exit to pid %d\n", pid);

cleanup:

  if(*path_to)
    unlink(path_to);
  if(*path_from)
    unlink(path_from);

  printf("\nexiting\n");
  return ret;
}
