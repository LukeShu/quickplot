/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */

#include <iostream>
#include <list>
#include <iomanip>

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
#include "GraphConfig.h"
#include "Globel.h"

#include "PlotLister.h"
#include "PlotConfig.h"
#include "StatusBar.h"
#include "MainMenuBar.h"
#include "ButtonBar.h"
#include "MainWindow.h"
#include "App.h"

#include "Source.h"
#include "FileList.h"
#include "File.h"

// There should be only one app.
App *app = NULL;
MainWindow *currentMainWindow = NULL;

// Calling parseArgs() in this constructor initialization is a trick
// to get the static variable app set and get some options set before
// mainWindow is created.
App::App(int *argc, char ***argv):
  Main(argc, argv)
{
  isInvalid = true;
  currentMainWindow = NULL;

  if(app) return; // error there can only be one app
  app = this;
  
  if(parseArgs1(*argc, *argv))
    return; // failure

  createMainWindow();
 
  if(parseArgs2(*argc, *argv))
    return; // failure
  
  if(opShowGraphConfig)
    currentMainWindow->showGraphConfig();
 
  isInvalid = false;
}

void App::_createMainWindow(bool makeGraph)
{
  MainWindow *mainWindow = new MainWindow(makeGraph);
  // add it to the list.
  push_back(mainWindow);
  
  mainWindow->show();

  if(!currentMainWindow)
    currentMainWindow = mainWindow;
 
  if(size() == 2)
  {
    // for size == 3 and higher the deleteFrameMenuItem sensitive is
    // set to true in MainMenuBar.cpp.
    std::list<MainWindow *>::const_iterator win = begin();
    for(;win != end(); win++)
    {
      (*win)->menuBar.deleteFrameMenuItem.set_sensitive(true);
    }
  }
}


// create a main window and add it to the list.
void App::createMainWindow(void)
{
  _createMainWindow(true);
}

void App::copyCurrentMainWindow(void)
{
  _createMainWindow(false);

  // copy the Graphs
  back()->graphsNotebook.copy(&(currentMainWindow->graphsNotebook));

  // copy width and height
  if(back()->get_width() != currentMainWindow->get_width() ||
     back()->get_height() != currentMainWindow->get_height())
  {
    back()->resize(currentMainWindow->get_width(), currentMainWindow->get_height());
  }

  // copy what is showing
  if(back()->menuBar.is_visible() != currentMainWindow->menuBar.is_visible())
  {
    if(currentMainWindow->menuBar.is_visible())
      back()->menuBar.show();
    else
      back()->menuBar.hide();
  }
  if(back()->buttonBar.is_visible() != currentMainWindow->buttonBar.is_visible())
  {
    if(currentMainWindow->buttonBar.is_visible())
      back()->buttonBar.show();
    else
      back()->buttonBar.hide();
  }
  if(back()->graphsNotebook.get_show_tabs() !=
     currentMainWindow->graphsNotebook.get_show_tabs())
  {
    back()->graphsNotebook.set_show_tabs(currentMainWindow->graphsNotebook.get_show_tabs());
  }
  if(back()->statusBar.is_visible() != currentMainWindow->statusBar.is_visible())
  {
    if(currentMainWindow->statusBar.is_visible())
      back()->statusBar.show();
    else
      back()->statusBar.hide();
  }
}


void App::destroyMainWindow(MainWindow *mainWindow)
{
  if(size() > 1)
  {
    if(mainWindow == currentMainWindow)
    {
      // set a different currentMainWindow.
      std::list<MainWindow *>::const_iterator win = begin();
      for(;win != end(); win++)
      {
        if(*win != mainWindow)
        {
          currentMainWindow = *win;
          break;
        }
      }
    }
  }
  else
  {
    quit();
    return;
  }

  if(opVerbose)
    opSpew << "Quickplot INFO: removing main window \""
           << mainWindow->get_title() << "\"" << std::endl;
  
  remove(mainWindow);

  if(mainWindow)
    delete mainWindow;

  if(size() <= 1)
  {
    currentMainWindow->menuBar.deleteFrameMenuItem.set_sensitive(false);
  }
}


extern "C"
{
  int *dummyData;
  
  static gboolean quitLater(gpointer data)
  {
    gtk_idle_remove_by_data(data);
    Main::quit();
    return ((gint) 0);
  }
}

void App::quit(void)
{
  // This seems to do the trick.
  gtk_idle_add(quitLater, dummyData);
}

App::~App(void)
{
 if(size() > 0)
  {
    std::list<MainWindow *>::const_iterator win = begin();
    for(;win != end(); win++)
      delete *win;
    clear(); // empty the list.
  }
  
  // delete all the source
  while(sources.size() > 0)
  {
    delete (*(sources.begin()));
  }
  sources.clear();
  app = NULL;
  
  //printf("line=%d file=%s\n",__LINE__, __FILE__);
}

// This gets a full path file name and then calls openFile if it can.
void App::openDialog(void)
{
  FileSelection dialog("Choose a Data File to Open");
  dialog.set_transient_for(*currentMainWindow);
  
  switch(dialog.run())
  {
    case(RESPONSE_OK):
      openFile(dialog.get_filename().c_str());
      break;
    default:
      break; // Closed window or hit cancel.
  }
}

void App::openFile(const char *filename)
{
  using std::endl;
  File *file = new File(filename);

  if(!file->isValid)
  {
    if(!opSilent)
      opSpew << "Failed to open " << filename <<  endl
             << errorStr << endl;
    // reset the error string
    errorStr[0] = '\0';
    delete file;
    return;
  }

  if(!opNoDefaultPlots && file->size() > 1 && opMaxNumDefaultPlots > 0)
  {
    Graph *graph = NULL;

    // find the first empty Graph in the graphsNotebook.
    int n = currentMainWindow->graphsNotebook.get_n_pages();
    int i;
    for(i=0;i<n;i++)
    {
      graph = dynamic_cast<Graph *>
        (currentMainWindow->graphsNotebook.get_nth_page(i));
      if(graph && graph->size() < 1)
        break;
      else
        graph = NULL;
    }

    if(!graph)
    {
      currentMainWindow->makeNewGraphTab();
      graph = currentMainWindow->currentGraph;
    }
    else
    {
      currentMainWindow->graphsNotebook.
        set_current_page(currentMainWindow->graphsNotebook.page_num(*graph));
    }
    
    
    graph->createDefaultPlots(file);
  }
  else
  {
    currentMainWindow->showGraphConfig();
    
    // Calling hide() before show() will make it visible even if the
    // window was icon-ified. show() alone will not cause the window to
    // be visible if it is icon-ified.  We think "blinking" the window
    // is better than not seeing it some times.  There does not appear
    // to be a function gboolean gtk_window_get_iconified(GtkWindow
    // *window), or a corrisponding method in GTKmm.
    currentMainWindow->graphConfig->hide();
    currentMainWindow->graphConfig->show();
  }

}
