/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */

#include <stdio.h>
#include <iostream>
#include <values.h>
#include <stdlib.h>

#include <gtkmm.h>


using namespace Gtk;
#include "MainMenuBar.h"
#include "ButtonBar.h"
#include "StatusBar.h"


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
#include "MainWindow.h"
#include "App.h"
#include "Source.h"
#include "Globel.h"

#include "quickplot_icon.xpm"


// from file: help_html.cpp which was generated from help.html.in
extern const char *help_html;

// from file: about_html.cpp which was generated from about.html.in
extern const char *about_html;


// From file: launchBrowser.cpp
extern int sLaunchBrowser(const char *doc, const char *suffix);

// from file:  parseGeometry.cpp
extern void parseGeometry(const char *geometry,
                   int &w, int &h,
                   int &x, int &y,
                          int &xSign, int & ySign);

#define SMALL_INT  INT_MIN


static int mainWindowCount = 0;


MainWindow::MainWindow(bool makeGraph):
  graphsNotebook(this),
  menuBar(this),
  buttonBar(this)
{
  mainWindowNumber = ++mainWindowCount;
  
  graphConfig = NULL;
  plotLister = NULL;
  numPlotConfigs = 0;
  
  startX = SMALL_INT;
  
  setTitle(NULL);
  
  graphTabAddCount = 0;

  if(!opShowGraphTabs)
    graphsNotebook.set_show_tabs(false);

  if(makeGraph)
  {
    makeNewGraphTab();
  }
  
  graphsNotebook.
    signal_switch_page().
    connect(SigC::slot(*this, &MainWindow::on_notebookFlip));

  
  add(mainVBox);
  mainVBox.pack_start(topVBox, PACK_SHRINK);
  topVBox.add(menuBar);
  topVBox.add(buttonBar);
  mainVBox.add(graphsNotebook);
  mainVBox.pack_start(statusBar, PACK_SHRINK);

  set_default_size(600, 600);
  
  Glib::RefPtr<Gdk::Pixbuf> pix =
    Gdk::Pixbuf::create_from_xpm_data(quickplot_icon);
  set_icon(pix);

  Source::signal_addedSource().
    connect(SigC::slot(*this, &MainWindow::setTitle));
  Source::signal_removedSource().
    connect(SigC::slot(*this, &MainWindow::setTitle));

  signal_hide().connect(SigC::slot(*this, &MainWindow::deleteLater));
  

  if(opGeometry && mainWindowNumber == 1) // example: -geometry 800x240+300+200
  {
    int w=SMALL_INT, h;
    // This will set the values only if they are valid.
    parseGeometry(opGeometry, w, h, startX, startY, startXSign, startYSign);
    if(w != SMALL_INT)
      resize(w, h);
  }

  // add a close menu item for each source.
  if(sources.size() > 0)
  {
    std::list<Source *>::const_iterator source = sources.begin();
    for(;source != sources.end(); source++)
    {
      (new CloseSourceMenuItem((*source), this))->show();
    }
  }

  {
    int i;
    for(i=0;i<NUMPLOTCONFIGS;i++)
    {
      // plotConfigXY[] is used to remember PlotConfig window
      // positions.  We set them to a value used to mean that it's not
      // set.
      plotConfigXY[0][i] = PC_SMALL_INT;
      plotConfigXY[1][i] = PC_SMALL_INT;
    }
  }
  
  Graph::signal_removedPlot().
    connect(SigC::slot(*this, &MainWindow::on_removedPlot));
}

void MainWindow::savePNGFile(void)
{
  FileSelection dialog("Choose a Filename to Save the PNG Image as:");
  dialog.set_transient_for(*this);
  
  switch(dialog.run())
  {
    case(RESPONSE_OK):
      currentGraph->savePNG(dialog.get_filename().c_str());
      break;
    default:
      break; // Closed window or hit cancel.
  }

}

bool MainWindow::on_expose_event(GdkEventExpose *e)
{
  if(startX != SMALL_INT && mainWindowNumber == 1)
  {
    // This is done here because I can't call
    // get_window()->get_frame_extents() before now.
    
    // set position to startX and startY
    if(startXSign < 0 || startYSign < 0)
    {
      Gdk::Rectangle rec;
      get_window()->get_frame_extents(rec);
      //opSpew << "rec.get_width()=" << rec.get_width()
      //     << " rec.get_height()=" << rec.get_height()
      //     << std::endl;
      
      int rootW, rootH;
      get_window()->get_parent()->get_size(rootW, rootH);
      //opSpew << "rootW=" << rootW << " rootH=" << rootH << std::endl;
      //opSpew << "startX=" << ((startXSign < 0)?"-":"+") << startX
      //       << " startY=" << ((startYSign < 0)?"-":"+") << startY
      //       << std::endl;
      if(startXSign < 0)
        startX = rootW - rec.get_width() - startX;
      if(startYSign < 0)
        startY = rootH - rec.get_height() - startY;

      //opSpew << "startX=" << startX
      //       << " startY=" << startY
      //      << std::endl;
    }

    move(startX, startY);
    startX = SMALL_INT;
  }

  return Window::on_expose_event(e);
}

void MainWindow::show(void)
{
  if(opShowMenuBar)
    menuBar.show();
  
  if(opShowButtons)
    buttonBar.show();
  
  graphsNotebook.show();
  topVBox.show();
  mainVBox.show();

  if(opShowStatusBar)
    statusBar.show();
  
  Window::show();
}

// looks at the files loaded and sets the title.
void MainWindow::setTitle(Source *) // slot
{
  char s[24];
  
  if(mainWindowNumber > 1)
  {
    sprintf(s, "(%d) ", mainWindowNumber);
  }
  else
    s[0] = '\0';

  sprintf(&s[strlen(s)], "Quickplot");
  
  Glib::ustring str(s);
  std::list<Source *>::const_iterator source = sources.begin();
  for(;source != sources.end(); source++)
  {
    if((*source)->getBaseFileName())
    {
      str += " ";
      str += (*source)->getBaseFileName();
    }
  }
 set_title(str);
}


void MainWindow::on_help(void)
{
  //opSpew << "MainWindow::on_help()" << std::endl;
  sLaunchBrowser(help_html, "help.html");
}

void MainWindow::on_about(void)
{
  //opSpew << "MainWindow::on_about()" << std::endl;
  sLaunchBrowser(about_html, "about.html");
}


void MainWindow::showGraphConfig(void)
{
  if(!graphConfig)
  {
    graphConfig = new GraphConfig(this);
  }
  graphConfig->show();
}

void MainWindow::showPlotLister(void)
{
  if(!plotLister)
  {
    plotLister = new PlotLister(this);
  }
  plotLister->show();
}


void MainWindow::makeNewGraphTabWithGraph(Graph *graph)
{
  graphTabAddCount++;
  graph->show();
  graphsNotebook.add(*graph);
  GraphTab *graphTab = new GraphTab(graphTabAddCount, this, graph);
  graphsNotebook.set_tab_label(*graph, *graphTab);
  graphsNotebook.set_current_page(graphsNotebook.page_num(*graph));
  currentGraph = graph;

  if(graphsNotebook.get_n_pages() == 2)
  {
    GraphTab *tab = dynamic_cast<GraphTab *>
      (graphsNotebook.get_tab_label(*graphsNotebook.get_nth_page(0)));
    if(tab)
    {
      tab->removeButton.set_sensitive(true);
    }
  }
  else if(graphsNotebook.get_n_pages() == 1)
  {
    GraphTab *tab = dynamic_cast<GraphTab *>
      (graphsNotebook.get_tab_label(*graphsNotebook.get_nth_page(0)));
    if(tab)
    {
      tab->removeButton.set_sensitive(false);
    }
  }
}


void MainWindow::makeNewGraphTab(void)
{
  makeNewGraphTabWithGraph(new Graph(this));
}

void MainWindow::makeNewGraphWithGraphConfig(void)
{
  makeNewGraphTab();
  showGraphConfig();
    
  // Calling hide() before show() will make it visible even if the
  // window was icon-ified. show() alone will not cause the window to
  // be visible if it is icon-ified.  We think "blinking" the window
  // is better than not seeing it some times.  There does not appear
  // to be a function gboolean gtk_window_get_iconified(GtkWindow
  // *window), or a corrisponding method in GTKmm.
  graphConfig->hide();
  graphConfig->show();
}


void MainWindow::removeGraphTab(Graph *graph)
{
  // remove Graph and Notebook page.
  int n = graphsNotebook.get_n_pages();

  // This will not remove the last graph.
  if(n <= 1) return;
  
  int i;
  for(i=0;i<n;i++)
  {
    Widget *g = graphsNotebook.get_nth_page(i);

    if(dynamic_cast<Graph *>(g) == graph)
    {
      Widget *label = graphsNotebook.get_tab_label(*graph);
      graphsNotebook.remove_page(i);
      delete graph;
      delete label;
      break;
    }
  }

  if(graphsNotebook.get_n_pages() == 1)
  {
    GraphTab *tab = dynamic_cast<GraphTab *>
      (graphsNotebook.get_tab_label(*graphsNotebook.get_nth_page(0)));
    if(tab)
    {
      tab->removeButton.set_sensitive(false);
    }
  }
}


void MainWindow::on_notebookFlip(GtkNotebookPage* page, guint page_num)
{
  // printf("MainWindow::on_notebookFlip(%d)\n", page_num);
  currentGraph = dynamic_cast<Graph *>(graphsNotebook.get_nth_page(page_num));
}

bool MainWindow::on_focus_in_event(GdkEventFocus* event)
{
  currentMainWindow = this;
  return Window::on_focus_in_event(event);
}

//opSpew << __LINE__ << " file=" << __FILE__ << std::endl;

MainWindow::~MainWindow(void)
{
// remove and delete a close menu item for each source.
  std::list<Source *>::const_iterator source = sources.begin();
  for(;source != sources.end(); source++)
  {
    bool got_one = true;
    while(got_one)
    {
      got_one = false;
      
      std::list<CloseSourceMenuItem *>::iterator it =
        (*source)->closeSourceMenuItems.begin();
      for(;it != (*source)->closeSourceMenuItems.end(); it++)
      {
         
        if((*it)->mainWindow == this)
        {
          // remove the close file menu
          menuBar.getFileMenu().items().remove(*(*it));
          // delete the close file menu
          delete (*it);
          // remove is from the list in source
          (*source)->closeSourceMenuItems.remove(*it);
          got_one = true;
          break;
        }
      }
      
    } // while(got_one)

    
  }
  

  int i;
  for(i=0;i<numPlotConfigs;i++)
  {
    delete plotConfigs[i];
  }
  numPlotConfigs = 0;
  
  if(graphConfig)
  {
    delete graphConfig;
    graphConfig = NULL;
  }
  
  if(plotLister)
  {
    delete plotLister;
    plotLister = NULL;
  }
  

  // remove all Graphs and Notebook pages.
  int n = graphsNotebook.get_n_pages();

  for(i=n-1;i>=0;i--)
  {
    Widget *graph = graphsNotebook.get_nth_page(i);
    Widget *label = graphsNotebook.get_tab_label(*graph);
    graphsNotebook.remove_page(i);
    delete graph;
    delete label;
  }
}

extern "C"
{
  struct DeleteLater
  {
    MainWindow *mainWindow;
  };
  
  static gboolean deleteThisLater(gpointer data)
  {
    gtk_idle_remove_by_data(data);
    app->destroyMainWindow(((struct DeleteLater *) data)->mainWindow);
    //opSpew << __LINE__ << " file=" << __FILE__ << std::endl;
    free(data);
    return ((gboolean) 0);
  }
}

void MainWindow::deleteLater(void)
{  
  struct DeleteLater *d = (struct DeleteLater *) malloc(sizeof(struct DeleteLater));
  d->mainWindow = this;
  gtk_idle_add(deleteThisLater, d);
}

void MainWindow::makePlotConfig(Plot *plot)
{
  {
    int i;
    for(i=0;i<numPlotConfigs;i++)
    {
      if(plotConfigs[i]->plot == plot)
      {
        plotConfigs[i]->show();
        return;
      }
    }
  }

  if(numPlotConfigs >= NUMPLOTCONFIGS)
  {
    // We have enough.  We convert the last one to the requested one.
    plotConfigs[NUMPLOTCONFIGS-1]->setPlot(plot);
    plotConfigs[NUMPLOTCONFIGS-1]->show();
    return;
  }

  plotConfigs[numPlotConfigs] =
    new PlotConfig(this, plot,
                   plotConfigXY[0][numPlotConfigs] /* x */,
                   plotConfigXY[1][numPlotConfigs] /* y */); 
  plotConfigs[numPlotConfigs]->show();
  numPlotConfigs++;
}

// Look for PlotConfig windows to remove, because a plot was just
// deleted.
void MainWindow::on_removedPlot(Graph *graph, Plot *plot)
{
  int i;
  for(i=0;i<numPlotConfigs;i++)
  {
    if((plotConfigs[i])->plot == plot)
    {
      deletePlotConfig(plotConfigs[i]);
      return;
    }
  }
}

void MainWindow::deletePlotConfig(PlotConfig *plotConfig)
{
  int i;  
  for(i=0;i<numPlotConfigs;i++)
  {
    if(plotConfigs[i] == plotConfig)
    {
      // save it's position for the next one made.  This will save the
      // user having to move a new one again, if he/she were to make
      // another.
      plotConfigXY[0][numPlotConfigs-1] = plotConfigs[i]->x;
      plotConfigXY[1][numPlotConfigs-1] = plotConfigs[i]->y;
     delete plotConfigs[i];
      break;
    }
  }
  

  if(i!=numPlotConfigs) // if we found and deleted it.
  {
    // Slide the remaining PlotConfigs over to the front of the
    // plotConfigs[] array.
    numPlotConfigs--;
    int j;
    for(j=i;j<numPlotConfigs;j++)
    {
      plotConfigs[j] = plotConfigs[j+1];
    }
  }
}


GraphsNotebook::GraphsNotebook(MainWindow *mainWindow_in):
  mainWindow(mainWindow_in)
{
  set_show_border(true);
}

void GraphsNotebook::set_show_tabs(bool show_tabs)
{
  Notebook::set_show_tabs(show_tabs);
  mainWindow->menuBar.checkGraphTabsState(show_tabs);

  // popup_enable() enables the popup menu: if the user clicks with
  // the right mouse button on the bookmarks, a menu with all the
  // pages will be popped up.  Appears to not work!!!
  // popup_enable();
}


extern "C"
{
  struct CopyGraphNotebooks
  {
    GraphsNotebook *to, *from;
  };
  
  static gboolean copyGraphNotebooks(gpointer data)
  {
    gtk_idle_remove_by_data(data);
    GraphsNotebook *to   = ((struct CopyGraphNotebooks *) data)->to;
    GraphsNotebook *from = ((struct CopyGraphNotebooks *) data)->from;
    to->_copyLater(from);
    free(data);
    return ((gint) 0);
  }
}


void GraphsNotebook::_copyLater(GraphsNotebook *graphsNotebook)
{
  int n = graphsNotebook->get_n_pages();
  int i;
  for(i=0;i<n;i++)
  {
    Graph *from = dynamic_cast<Graph *>(graphsNotebook->get_nth_page(i));
    Graph *to   = dynamic_cast<Graph *>(get_nth_page(i));
    to->copy(from);
  }
  int currentPageNum = graphsNotebook->get_current_page();
  set_current_page(currentPageNum);
  mainWindow->currentGraph = dynamic_cast<Graph *>(get_nth_page(currentPageNum));
  mainWindow->currentGraph->queueRedraw();
}

// copy from graphNotebook to this.
void GraphsNotebook::copy(GraphsNotebook *graphsNotebook)
{
  int n = graphsNotebook->get_n_pages();
  int i;
  for(i=0;i<n;i++)
  {
    mainWindow->makeNewGraphTab();
    GraphTab *toLabel = dynamic_cast<GraphTab *>
      (get_tab_label(*mainWindow->currentGraph));
    GraphTab *fromLabel = dynamic_cast<GraphTab *>
      (graphsNotebook->get_tab_label(*(graphsNotebook->get_nth_page(i))));
    
    if(toLabel && fromLabel)
      toLabel->setText(fromLabel->label.get_text().c_str());
  }

  // Set up to do the graph plot coping later, after the Graphs are
  // realized, so that the new plots can get the necessary widget
  // info.
  struct CopyGraphNotebooks *d = (struct CopyGraphNotebooks *)
    malloc(sizeof(struct CopyGraphNotebooks));
  d->to= this;
  d->from = graphsNotebook;
  gtk_idle_add(copyGraphNotebooks, d);
}
