/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */
#include <iostream>
#include <values.h>
#include <stdlib.h>

#include <gtkmm.h>


using namespace Gtk;
#include "value_t.h"
#include "Globel.h"

#include "ValueSlider.h"
#include "PlotLister.h"
#include "PlotConfig.h"
#include "MainMenuBar.h"
#include "ButtonBar.h"
#include "StatusBar.h"

#include "MainWindow.h"

#include "Field.h"
#include "Plot.h"
#include "ColorGen.h"
#include "Graph.h"
#include "PlotSelector.h"
#include "GraphConfig.h"
#include "App.h"
#include "Source.h"

#include "quickplot_icon.xpm"


PlotConfig::PlotConfig(MainWindow *w, Plot *p,
                       int x_in, int y_in):
  
  picture(p),

  lineFrame("Line Properties"),
  showLineCB("Showing"),
  lineColorB("Set Color"),
  lineWidthVS(0, MAX_PLOT_LINE_WIDTH, MAXMAX_PLOT_LINE_WIDTH, "Width"),
  
  pointFrame("Point Properties"),
  showPointsCB("Showing"),
  pointColorB("Set Color"),
  pointSizeVS(0, MAX_POINT_SIZE, MAXMAX_POINT_SIZE, "Size")
{
  mainWindow = w;
  plot = NULL;
  set_default_size(300, 200);
  picture.set_size_request(-1,30);
  
  vBox.set_spacing(8);
  hBox.set_spacing(8);
  lineVBox.set_border_width(8);
  pointVBox.set_border_width(8);
  

  add(vBox);
  vBox.pack_start(picture, PACK_EXPAND_WIDGET);
  vBox.pack_start(hBox, PACK_SHRINK);
  hBox.add(lineFrame);
  hBox.add(pointFrame);
  
  lineFrame.add(lineVBox);
  lineVBox.add(showLineCB);
  lineVBox.add(lineColorB);
  lineVBox.add(lineWidthVS);
  
  pointFrame.add(pointVBox);
  pointVBox.add(showPointsCB);
  pointVBox.add(pointColorB);
  pointVBox.add(pointSizeVS);
  
  setPlot(p);

  showLineCB.signal_clicked().
    connect( SigC::slot(*this, &PlotConfig::on_showLine));
  lineColorB.signal_pressed().
    connect( SigC::slot(*this, &PlotConfig::on_lineColor));
  lineWidthVS.signal_valueChanged().
    connect( SigC::slot(*this, &PlotConfig::on_lineWidth));
  
  showPointsCB.signal_clicked().
    connect( SigC::slot(*this, &PlotConfig::on_showPoints));
  pointColorB.signal_pressed().
    connect( SigC::slot(*this, &PlotConfig::on_pointColor));
  pointSizeVS.signal_valueChanged().
    connect( SigC::slot(*this, &PlotConfig::on_pointSize));

  signal_hide().connect(SigC::slot(*this,&PlotConfig::deleteLater));

  Glib::RefPtr<Gdk::Pixbuf> pix =
    Gdk::Pixbuf::create_from_xpm_data(quickplot_icon);
  set_icon(pix);

  x = x_in;
  y = y_in;

  show_all_children();
}

void PlotConfig::setTitle(void)
{
  char s[16];
  
  if(mainWindow->mainWindowNumber > 1)
  {
    sprintf(s, "(%d): ", mainWindow->mainWindowNumber);
  }
  else
    s[0] = '\0';
  
  Glib::ustring str = s;
  str += "Configure Plot: ";
  str += plot->getLabel();
  str += " : ";
  
  GraphTab *l = dynamic_cast<GraphTab *>
    (mainWindow->graphsNotebook.get_tab_label(*(mainWindow->currentGraph)));
  if(l)
    str += l->label.get_text();
  
  set_title(str);
}


void PlotConfig::on_unmap()
{
  // Remember where the window was positioned if it is not showing
  // now.
  get_position(x , y);
  Window::on_unmap();
}


void PlotConfig::show()
{
  // Remember where the window was positioned.
  if(!is_visible() && x != PC_SMALL_INT)
    move(x, y);
  
  Window::show();
}


void PlotConfig::setPlot(Plot *p)
{
  
  plot = p;

  // now modify widgets. ....
  setTitle();

  picture.setPlot(plot);
  if(is_visible())
    picture.queueRedraw();

  plotChanged_connection.disconnect();
  
  plotChanged_connection =
    plot->signal_changed().
    connect(SigC::slot(*this,&PlotConfig::on_plotChanged));
 
  setValues();
}

extern "C"
{
  struct DeleteLater
  {
    PlotConfig *plotConfig;
    MainWindow *mainWindow;
  };
  
  static gboolean deleteThisLater(gpointer data)
  {
    gtk_idle_remove_by_data(data);
    struct DeleteLater *d = (struct DeleteLater *) data;
    d->mainWindow->deletePlotConfig(d->plotConfig);
    free(data);
    return ((gint) 0);
  }
}


void PlotConfig::deleteLater(void)
{
  struct DeleteLater *d = (struct DeleteLater *)
    malloc(sizeof(struct DeleteLater));
  d->plotConfig = this;
  d->mainWindow = mainWindow;
  gtk_idle_add(deleteThisLater, d);
}

void PlotConfig::setValues(void)
{
  if(plot->getShowLines() != showLineCB.get_active())
    showLineCB.set_active(plot->getShowLines());
  lineWidthVS.setValue(plot->getLineWidth());

  
  if(plot->getShowPoints() != showPointsCB.get_active())
    showPointsCB.set_active(plot->getShowPoints());
  pointSizeVS.setValue(plot->getPointSize());
}

void PlotConfig::on_plotChanged(void)
{
  setValues();
}

void PlotConfig::on_showLine(void)
{
  if(plot->getShowLines() != showLineCB.get_active())
  {
    plot->setShowLines(showLineCB.get_active());
    plot->graph->queueRedraw();
    if(mainWindow->graphConfig)
      mainWindow->graphConfig->plotSelector.drawArea.queueRedraw();
  }
}

void PlotConfig::on_lineColor(void)
{
  Gtk::ColorSelectionDialog dialog;

  //Set the current color:
  Gtk::ColorSelection* pColorSel = dialog.get_colorsel();
  pColorSel->set_current_color(plot->getLineColor());

  { // set dialog title
    char s[16];
    if(mainWindow->mainWindowNumber > 1)
    {
      sprintf(s, "(%d): ", mainWindow->mainWindowNumber);
    }
    else
      s[0] = '\0';
    
    Glib::ustring str = s;
    str += "Plot Line Color: ";
    str += plot->getLabel();
    str += ": ";
    GraphTab *l = dynamic_cast<GraphTab *>
      (mainWindow->graphsNotebook.get_tab_label(*(mainWindow->currentGraph)));
    if(l)
      str += l->label.get_text();
    dialog.set_title(str);
  }
 
  int result = dialog.run();
  
  //Handle the response:
  switch(result)
  {
    case(Gtk::RESPONSE_OK):
    {
      const Gdk::Color color = pColorSel->get_current_color();
      plot->setLineColor(color);
      if(mainWindow->graphConfig)
        mainWindow->graphConfig->plotSelector.drawArea.queueRedraw();
      plot->graph->queueRedraw();
      break;
    }
    case(Gtk::RESPONSE_CANCEL):
    {
      //opSpew << "Cancel clicked." << std::endl;
      break;
    }
    default:
    {
      //opSpew << "Unexpected button clicked." << std::endl;
      break;
    }
  }
}

void PlotConfig::on_lineWidth(void)
{
  if(plot->getLineWidth() != lineWidthVS.getValue())
  {
    plot->setLineWidth(lineWidthVS.getValue());
    if(plot->getShowLines())
      plot->graph->queueRedraw();
    if(mainWindow->graphConfig)
      mainWindow->graphConfig->plotSelector.drawArea.queueRedraw();
  }
}

void PlotConfig::on_showPoints(void)
{
  if(plot->getShowPoints() != showPointsCB.get_active())
  {
    plot->setShowPoints(showPointsCB.get_active());
    plot->graph->queueRedraw();
    if(mainWindow->graphConfig)
      mainWindow->graphConfig->plotSelector.drawArea.queueRedraw();
  }
}

void PlotConfig::on_pointColor(void)
{
  Gtk::ColorSelectionDialog dialog;

  //Set the current color:
  Gtk::ColorSelection* pColorSel = dialog.get_colorsel();
  pColorSel->set_current_color(plot->getPointColor());
  
  { // set dialog title
    char s[16];
    if(mainWindow->mainWindowNumber > 1)
    {
      sprintf(s, "(%d): ", mainWindow->mainWindowNumber);
    }
    else
      s[0] = '\0';
    
    Glib::ustring str = s;
    str += "Plot Point Color: ";
    str += plot->getLabel();
    str += ": ";
    GraphTab *l = dynamic_cast<GraphTab *>
      (mainWindow->graphsNotebook.get_tab_label(*(mainWindow->currentGraph)));
    if(l)
      str += l->label.get_text();
    dialog.set_title(str);
  }

  int result = dialog.run();
  
  //Handle the response:
  switch(result)
  {
    case(Gtk::RESPONSE_OK):
    {
      const Gdk::Color color = pColorSel->get_current_color();
      plot->setPointColor(color);
      if(mainWindow->graphConfig)
        mainWindow->graphConfig->plotSelector.drawArea.queueRedraw();
      plot->graph->queueRedraw();
      break;
    }
    case(Gtk::RESPONSE_CANCEL):
    {
      //opSpew << "Cancel clicked." << std::endl;
      break;
    }
    default:
    {
      //opSpew << "Unexpected button clicked." << std::endl;
      break;
    }
  }
}

void PlotConfig::on_pointSize(void)
{
  if(plot->getPointSize() != pointSizeVS.getValue())
  {
    plot->setPointSize(pointSizeVS.getValue());
    if(plot->getShowPoints())
      plot->graph->queueRedraw();
    if(mainWindow->graphConfig)
      mainWindow->graphConfig->plotSelector.drawArea.queueRedraw();
  }
}

// Widget Key Accelerators don't work all the time, so we get the key
// strokes the hard way.  Also there is no way to use Widget Key
// Accelerators with Gtk::Window.
bool PlotConfig::on_key_press_event(GdkEventKey* event)
{
  // Can add checks on GdkModifierType in event->state.

  switch(event->keyval)
    {
    case GDK_Escape:
    case GDK_p:
      {
	hide();
	return true;
	break;
      }
    default:
      break;
    }

  return Window::on_key_press_event(event);
}
