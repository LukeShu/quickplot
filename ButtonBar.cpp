/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */
#include "config.h"

#include <list>

#ifdef QP_ARCH_DARWIN
# include <limits.h>
# include <float.h>
#else
# include <values.h>
#endif

#include <gtkmm.h>

using namespace Gtk;
#include "errorStr.h"
#include "value_t.h"
#include "Field.h"
#include "Plot.h"
#include "ColorGen.h"
#include "Graph.h"
#include "PlotSelector.h"
#include "ValueSlider.h"
#include "PlotLister.h"
#include "PlotConfig.h"
#include "GraphConfig.h"

#include "MainMenuBar.h"
#include "ButtonBar.h"
#include "StatusBar.h"
#include "MainWindow.h"
#include "App.h"

#include "Globel.h"


ButtonBar::ButtonBar(MainWindow *mainWindow_in):
  HButtonBox(BUTTONBOX_START), // makes all the buttons the same size.
  openButton("_Open File ...", true),
  newButton("_New Graph Tab", true),
  showGraphConfigButton(opShowGraphConfig?
                        "Hide Config":"Show Config", true),
  savePNGButton("Save PNG _Image ...", true)
{
  mainWindow = mainWindow_in;
  add(openButton);
  add(newButton);
  add(showGraphConfigButton);
  add(savePNGButton);

  openButton.signal_activate().connect(sigc::mem_fun(*app, &App::openDialog));
  openButton.signal_pressed().connect(sigc::mem_fun(*app, &App::openDialog));
  
  newButton.signal_activate().
    connect(sigc::mem_fun(*mainWindow,
                       &MainWindow::makeNewGraphWithGraphConfig));
  newButton.signal_pressed().
    connect(sigc::mem_fun(*mainWindow,
                       &MainWindow::makeNewGraphWithGraphConfig));
  savePNGButton.signal_pressed().connect(sigc::mem_fun(*mainWindow,
                                                    &MainWindow::savePNGFile));
  
  showGraphConfigButton.signal_activate().
      connect(sigc::mem_fun(*this,
                         &ButtonBar::on_showGraphConfigButton));
  showGraphConfigButton.signal_pressed().
      connect(sigc::mem_fun(*this,
                         &ButtonBar::on_showGraphConfigButton));
  
  signal_show().connect(sigc::mem_fun(mainWindow->menuBar,
                                   &MainMenuBar::checkButtonBarState));
  signal_hide().connect(sigc::mem_fun(mainWindow->menuBar,
                                   &MainMenuBar::checkButtonBarState));
  
  openButton.show();
  newButton.show();
  showGraphConfigButton.show();
  savePNGButton.show();
}

void ButtonBar::checkGraphConfigButton(void)
{
  if(mainWindow->graphConfig && mainWindow->graphConfig->is_visible() &&
     showGraphConfigButton.get_label() != "Hide Config")
    showGraphConfigButton.set_label("Hide Config");
  else if((!mainWindow->graphConfig || !mainWindow->graphConfig->is_visible()) &&
          showGraphConfigButton.get_label() != "Show Config")
    showGraphConfigButton.set_label("Show Config");
}

void ButtonBar::on_showGraphConfigButton(void)
{
  if(!mainWindow->graphConfig || !mainWindow->graphConfig->is_visible())
  {
    mainWindow->showGraphConfig();
  }
  else if(mainWindow->graphConfig && mainWindow->graphConfig->is_visible())
  {
    mainWindow->graphConfig->hide();
  }
}


FancyButton::FancyButton(const Glib::ustring& label, bool mnemonic) :
  Button(label, mnemonic)
{
  // plan to add more here
}
