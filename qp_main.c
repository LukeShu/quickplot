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

#include "quickplot.h"

#include "config.h"
#include "debug.h"
#include "spew.h"
#include "list.h"
#include "qp.h"


int main (int argc, char **argv)
{
  struct qp_gtk_options *gtk_opt;

  qp_app_create();

  gtk_opt = strip_gtk_options(&argc, &argv);

  qp_getargs_1st_pass(argc, argv);

  if(qp_gtk_init_check(gtk_opt)) return 1;

  qp_getargs_2nd_pass(argc, argv);

  gtk_main();

  return 0;
}
