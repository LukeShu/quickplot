/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */
#include "config.h"
#include <stdio.h>
#include <gtkmm.h>


using namespace Gtk;
#include "value_t.h"
#include "ValueSlider.h"
#include "PlotLister.h"
#include "PlotConfig.h"
#include "MainMenuBar.h"
#include "ButtonBar.h"
#include "StatusBar.h"
#include "MainWindow.h"
#include "App.h"



int main (int argc, char *argv[])
{
  App app(&argc, &argv);

  if(app.isInvalid) return 1;

  app.run(); //Shows the window and returns when it is closed.

  return 0;
}
