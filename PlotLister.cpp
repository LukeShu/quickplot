/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */

//#include <stdio.h>
#include <iostream>
#include <values.h>
#include <stdlib.h>

#include <gtkmm.h>


using namespace Gtk;
#include "value_t.h"
#include "PlotLister.h"
#include "MainMenuBar.h"
#include "ButtonBar.h"
#include "StatusBar.h"

#include "Field.h"
#include "Plot.h"
#include "ColorGen.h"
#include "Graph.h"
#include "PlotSelector.h"
#include "ValueSlider.h"
#include "PlotConfig.h"
#include "GraphConfig.h"
#include "MainWindow.h"
#include "App.h"
#include "Source.h"
#include "Globel.h"

#include "quickplot_icon.xpm"

#define SMALL_INT  INT_MIN


PlotLister::PlotLister(MainWindow *mainWindow_in):
  offPlotMI("Pick Pointer Values"),
  interpolatedMI("Interpolate Plot Values"),
  noninterpolatedMI("Pick Plot Point Values"),
  printToStdoutB("Print to Standard Output"),
  table(1,8),
  plotLabelL("Label"),
  plotPictureL("Color"),
  valuesL("X Y Values"),
  xMinL("X min"),
  xMaxL("X max"),
  yMinL("Y min"),
  yMaxL("Y max")
{
  mainWindow = mainWindow_in;
  set_default_size(630, 260);

  add(vBox);
  vBox.pack_start(hBox, PACK_SHRINK);
  hBox.set_spacing(10);
  scrolledWindow.set_policy(POLICY_AUTOMATIC, POLICY_AUTOMATIC);
  
  hBox.add(pickerTypeOM);
  hBox.add(printToStdoutB);
  
  vBox.add(scrolledWindow);
  scrolledWindow.add(table);
  addHeaderLabel(plotLabelL, 1);
  addHeaderLabel(plotPictureL, 2);
  addHeaderLabel(valuesL,  3);
  addHeaderLabel(xMinL, 4);
  addHeaderLabel(xMaxL, 5);
  addHeaderLabel(yMinL, 6);
  addHeaderLabel(yMaxL, 7);
  rowArraySize = 0;
  numRows = 0;
  row = NULL;


  // pickerType Menu
  {
    pickerTypeOM.set_menu(pickerTypeM);
    Menu::MenuList& menulist = pickerTypeM.items();
    
    menulist.push_back(offPlotMI);
    offPlotMI.show();
    offPlotMI.signal_activate().
      connect(SigC::slot(*this, &PlotLister::on_offPlot));
    
    menulist.push_back(interpolatedMI);
    interpolatedMI.show();
    interpolatedMI.signal_activate().
      connect(SigC::slot(*this, &PlotLister::on_interpolated));
    
    menulist.push_back(noninterpolatedMI);
    noninterpolatedMI.show();
    noninterpolatedMI.signal_activate().
      connect(SigC::slot(*this, &PlotLister::on_noninterpolated));
  }

  // We just use the closeButton to get <escape> to close the window.
  // We don't show the closeButton.
  closeButton.signal_activate().connect(SigC::slot(*this, &PlotLister::hide));
  closeButton.add_accelerator("activate", get_accel_group(),
                              GDK_Escape, Gdk::LOCK_MASK, ACCEL_MASK);

  show_all_children();
  
  
  Glib::RefPtr<Gdk::Pixbuf> pix =
    Gdk::Pixbuf::create_from_xpm_data(quickplot_icon);
  set_icon(pix);

  
  setValuesFromGraph();

  signal_show().connect(SigC::slot(mainWindow->menuBar,
                                   &MainMenuBar::checkPlotListerState));
  signal_hide().connect(SigC::slot(mainWindow->menuBar,
                                   &MainMenuBar::checkPlotListerState));

  mainWindow->graphsNotebook.
    signal_switch_page().
    connect(SigC::slot(*this, &PlotLister::on_notebookFlip));


  Graph::signal_removedPlot().
    connect(SigC::slot(*this, &PlotLister::on_removedPlot));
  Graph::signal_addedPlot().
    connect(SigC::slot(*this, &PlotLister::on_addedPlot));

  printToStdoutB.signal_pressed().
    connect(SigC::slot(*this, &PlotLister::on_printToStdout));
                 

  x = SMALL_INT;
}

void PlotLister::on_printToStdout(void)
{
  if(numRows)
  {
    unsigned int i;
    for(i=0;i<numRows; i++)
      row[i]->printStdout();
  }
}


void PlotLister::addHeaderLabel(Widget &w, int col)
{
  table.attach(w, col, col+1, 0, 1,
               SHRINK|FILL|EXPAND, SHRINK|FILL, 2, 2);
}

PlotLister::~PlotLister(void)
{
  if(numRows)
  {
    unsigned int i;
    for(i=0;i<numRows; i++)
      delete row[i];
    free(row);
  }
}

void PlotLister::on_notebookFlip(GtkNotebookPage* , guint )
{
  setValuesFromGraph();
}

void PlotLister::setTabAsTitle(void)
{
  char s[16];
  
  if(mainWindow->mainWindowNumber > 1)
  {
    sprintf(s, "(%d): ", mainWindow->mainWindowNumber);
  }
  else
    s[0] = '\0';
  
  Glib::ustring str = s;
  
  GraphTab *l = dynamic_cast<GraphTab *>
    (mainWindow->graphsNotebook.get_tab_label(*(mainWindow->currentGraph)));
  if(l)
    str += l->label.get_text();
  
  str += ": Plot List";
  set_title(str);

  //str = "Plot List: ";
  //str += l->label.get_text();
  
  // topLabel.set_title(str);
}

void PlotLister::on_addedPlot(Graph *graph)
{
  if(graph == mainWindow->currentGraph)
    setValuesFromGraph();
}

void PlotLister::on_removedPlot(Graph *graph, Plot *plot)
{
  if(graph == mainWindow->currentGraph)
    setValuesFromGraph();
}

void PlotLister::setValuesFromGraph(void)
{
  setTabAsTitle();
  
  interpolatedMI.
    set_sensitive(mainWindow->currentGraph->highestPickerType >=
                  Graph::INTERPOLATED);
  
  noninterpolatedMI.
    set_sensitive(mainWindow->currentGraph->highestPickerType >=
                  Graph::NONINTERPOLATED);
    
  pickerTypeM.set_active(mainWindow->currentGraph->pickerType);
  pickerTypeOM.set_history(mainWindow->currentGraph->pickerType);

  
    // delete the old widgets
  if(numRows)
  {
    unsigned int i;
    for(i=0;i<numRows; i++)
      delete row[i];
    numRows = 0;
  }
  
  Graph *graph = mainWindow->currentGraph;
    
  if(rowArraySize < graph->size())
  {
    rowArraySize = graph->size();
    if(row) free(row);
    row = static_cast<Row **>(malloc(sizeof(Row *)*rowArraySize));
  }

  numRows = graph->size();
  table.resize(numRows+1, 7);
  
  std::list<Plot *>::const_iterator p = graph->begin();
  unsigned int i=0;
  for( ;i<numRows; p++)
  {
    row[i] = new Row(mainWindow, &table, i+1, *p);
    i++;
  }
}


void PlotLister::on_unmap()
{
  // Remember where the window was positioned if it is not showing
  // now.
  get_position(x , y);
  Window::on_unmap();
}

void PlotLister::show()
{
  // Remember where the window was positioned.
  if(!is_visible() && x != SMALL_INT)
    move(x, y);
  
  Window::show();
}

void PlotLister::on_interpolated(void)
{
  mainWindow->currentGraph->pickerType = Graph::INTERPOLATED;
}

void PlotLister::on_noninterpolated(void)
{
  mainWindow->currentGraph->pickerType = Graph::NONINTERPOLATED;
}

void PlotLister::on_offPlot(void)
{
  mainWindow->currentGraph->pickerType = Graph::OFF_PLOT;
}


Row::Row(MainWindow *mainWindow_in, Table *table_in, int row_in, Plot *plot_in):
  plotConfigB("Configure Plot"),
  picture(plot_in, true)
{
  mainWindow = mainWindow_in;
  plot = plot_in;
  
  row = row_in;
  table = table_in;
  
  labelE.set_size_request(200,-1);
  picture.set_size_request(60,-1);
  valueE.set_size_request(300,-1);
  
  labelE.set_text(plot->getLabel());
  if(plot->xpick != MAXVALUE)
    on_valueDisplay(plot->xpick, plot->ypick);
  else
    valueE.set_text("x y");
  char s[32];
  sprintf(s, FORMAT, plot->xmin);
  xMinE.set_text(s);
  sprintf(s, FORMAT, plot->xmax);
  xMaxE.set_text(s);
  sprintf(s, FORMAT, plot->ymin);
  yMinE.set_text(s);
  sprintf(s, FORMAT, plot->ymax);
  yMaxE.set_text(s);

  add(plotConfigB, 0);
  add(labelE, 1);
  add(picture, 2);
  add(valueE, 3);
  add(xMinE, 4);
  add(xMaxE, 5);
  add(yMinE, 6);
  add(yMaxE, 7);

  plot->signal_valueDisplay().
    connect(SigC::slot(*this, &Row::on_valueDisplay));
  plotConfigB.signal_pressed().
    connect(SigC::slot(*this, &Row::makePlotConfig));
}

void Row::makePlotConfig(void)
{
  mainWindow->makePlotConfig(plot);
}

void Row::add(Widget &w, int col)
{
  w.show();
  table->attach(w, col, col+1, row, row+1,
                SHRINK|FILL|EXPAND, SHRINK|FILL, 2, 2);
}

void Row::on_valueDisplay(value_t x, value_t y)
{
  char s[64];
  snprintf(s,64, FORMAT"  "FORMAT, x, y);
  valueE.set_text(s);
}

void Row::printStdout(void)
{
  printf("%s\n", valueE.get_text().c_str());
}


Picture::Picture(Plot *plot_in, bool limitSize_in)
{
  limitSize = limitSize_in;
  win.clear();
  gc.clear();

  setPlot(plot_in);
}

Picture::~Picture(void)
{
}

bool Picture::on_expose_event(GdkEventExpose *)
{
  if(plot)
  {
    if(win.is_null())
    {
      win = get_window();
      gc = Gdk::GC::create(win);
    }
    
    win->set_background(plot->graph->backgroundColor);
    win->clear();

    if(plot->getShowLines()) 
    {
      // draw lines
      
      int lineWidth = plot->getLineWidth();
      if(limitSize && lineWidth > 11)
        lineWidth = 11;
      
      gc->set_foreground(plot->getLineColor());
      
      gc->set_line_attributes (lineWidth, Gdk::LINE_SOLID,
                               Gdk::CAP_ROUND, Gdk::JOIN_ROUND);
      
      win->draw_line(gc, 0, 2*get_height()/3,
                     get_width()/3, get_height()/3);
      win->draw_line(gc, get_width()/3, get_height()/3,
                     2*get_width()/3, get_height()/3);
      win->draw_line(gc, 2*get_width()/3, get_height()/3,
                     get_width(), 2*get_height()/3);
    }
    
    if(plot->getShowPoints())
    {
      // put 2 points in the lines
      
      int pointSize = plot->getPointSize();
      if(limitSize && pointSize > 11)
        pointSize = 11;

      
      gc->set_foreground(plot->getPointColor());
      
      if(pointSize <= 1)
      {
        win->draw_point(gc, get_width()/3, get_height()/3);
        win->draw_point(gc, 2*get_width()/3, get_height()/3);
      }
      else
      {
        win->draw_rectangle(gc, true,
                            get_width()/3 - pointSize/2,
                            get_height()/3 - pointSize/2,
                            pointSize, pointSize);
        win->draw_rectangle(gc, true,
                            2*get_width()/3 - pointSize/2,
                            get_height()/3 - pointSize/2,
                            pointSize, pointSize);
      }
    }
  }
  
 return true;
}

void Picture::queueRedraw(void)
{
  GdkRectangle rec =
    {
      0, 0, get_width(), get_height()
    };
  
  gdk_window_invalidate_rect(get_window()->gobj(), &rec, true);
}

void Picture::setPlot(Plot *p)
{
  if(plot)
  {
    plotChangeConnection.disconnect();
    bgConnection.disconnect();
  }

  plot = p;

  if(plot)
  {
    plotChangeConnection =
      plot->signal_changed().connect(
      SigC::slot(*this, &Picture::queueRedraw));

    bgConnection =
      plot->graph->signal_backgroundColorChanged().
      connect(SigC::slot(*this, &Picture::queueRedraw));
  }
}

