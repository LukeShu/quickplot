/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */

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

StatusBar::StatusBar(void)
{
  positionEntry.set_size_request(300,-1);
  
  pack_start(positionEntry, PACK_SHRINK);
  positionEntry.set_editable(false);

  show_all_children();
}

