/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */
#include "config.h"

#include <list>
#include <values.h>

#include <gtkmm.h>
#ifdef USE_LIBSNDFILE
#  include <sndfile.h>
#endif


using namespace Gtk;
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
#include "Source.h"

#define SMALL_INT  INT_MIN
#define ZRO        ((value_t) 0.0)
#define ABSVAL(x)  (((x) > 0)? (x) : (-(x)))



Graph::Graph(MainWindow *mainWindow_in)
{
  mainWindow = mainWindow_in;
  
  isDeleting = false;

  pickerType = INTERPOLATED;
  highestPickerType = NONINTERPOLATED;
  
  // minimum size
  set_size_request(20, 20);

  win.clear();
  gc.clear();

  set_events(Gdk::EXPOSURE_MASK|
             Gdk::POINTER_MOTION_MASK|
             Gdk::BUTTON_PRESS_MASK|
             Gdk::BUTTON_RELEASE_MASK);

  xPress = yPress = xmouse = ymouse = xpick = ypick = SMALL_INT;

  // Do the current plots have the same scale.
  isSameScale = true;
  
  // Do we force all plots to have the same scale.
  sameScale = opSameScale;
  
   // Do we make all plots the same scale if it works well.
  autoSameScale = opAutoSameScale;
  
  showAutoGrid  = opShowAutoGrid;
  showGridNumbers = true;

  gridLineWidth = opGridLineWidth;
  gridXLineSpace = opGridXLineSpace;
  gridYLineSpace = opGridYLineSpace;


  // defaults for plots
  showLines  = opShowLines;
  showPoints = opShowPoints;
  lineWidth   = opLineWidth;
  pointSize  = opPointSize;

  
  checkScalesQueued = false;


  // We'll have to see about how to get a theme color here.
  // set the default colors.
  gushort r, g, b;
  colorGen.getBackgroundColor(r, g, b);
  backgroundColor.set_red(r);
  backgroundColor.set_green(g);
  backgroundColor.set_blue(b);

  colorGen.getGridColor(r, g, b);
  gridColor.set_red(r);
  gridColor.set_green(g);
  gridColor.set_blue(b);
  
  pangolayout.clear(); // set to null.
  
  lastPickerType = NONE;
  buttonPressed = 0;

  // old_width and old_height are used for seeing if there is a
  // resize.
  old_width = old_height = SMALL_INT;
}

// a copy the plots and properties from a graph.
void Graph::copy(Graph *graph)
{
  pickerType = graph->pickerType;
  highestPickerType = graph->highestPickerType;
  
  isSameScale = graph->isSameScale;
  sameScale = graph->sameScale;
  autoSameScale = graph->autoSameScale;
  
  showAutoGrid  = graph->showAutoGrid;
  showGridNumbers = graph->showGridNumbers;

  setGridColor(graph->gridColor);
  setBackgroundColor(graph->backgroundColor);
  
  gridLineWidth = graph->gridLineWidth;
  gridXLineSpace = graph->gridXLineSpace;
  gridYLineSpace = graph->gridYLineSpace;

  
  // defaults for plots
  showLines  = graph->showLines;
  showPoints = graph->showPoints;
  lineWidth   = graph->lineWidth;
  pointSize  = graph->pointSize;
  
  
  checkScalesQueued = graph->checkScalesQueued;

  // copy all the plots
  std::list<Plot *>::const_iterator plot = graph->begin();
  for(;plot != graph->end(); plot++)
  {
    new Plot(this, *plot);
  }
}

Graph::~Graph(void)
{
  isDeleting = true;
  
  // remove and delete all the plots.
  while(size() > 0)
  {
    delete (*begin()); // This will remove it from the list too.
  }
}
    
 
// create and add a Plot to the list.
Plot *Graph::createPlot(Field *x, Field *y)
{
  Plot *plot = new Plot(this, x, y); // Plot adds itself to the
                                     // list.
  checkZoomLevelForNewPlot(plot);
  setPickerType();

  if(size() == 1)
  {
    GraphTab *tab = dynamic_cast<GraphTab *>
      (mainWindow->graphsNotebook.get_tab_label(*this));
    if(tab)
      {
	tab->setText(y->source->getBaseFileName());
	mainWindow->graphsNotebook.m_signal_tabLabelChanged.emit(this);
      }
  }
  
  m_signal_addedPlot.emit(this);

  return plot;
}

void Graph::setPickerType(void)
{

  highestPickerType = NONINTERPOLATED;

  if(size() > 0)
  {
    std::list<Plot *>::const_iterator p = begin();
    Field *X = (*p)->x();
    count_t numValues = (*p)->getNumberOfPoints();
    
    if(!((*p)->x()->isIncreasing()))
    {
      pickerType = highestPickerType = OFF_PLOT;
      return;
    }
    
    for(;p != end(); p++)
    {
      if(!((*p)->x()->isIncreasing()))
      {
        // We will not try to pick values in a nonFunction plot.
        pickerType = highestPickerType = OFF_PLOT;
        return;
      }
      if((*p)->x() != X || numValues != (*p)->getNumberOfPoints())
      {
        // We will not try to pick plot points if all the plots do not
        // use the same X values.
        highestPickerType = INTERPOLATED;
        if(pickerType > highestPickerType)
          pickerType = highestPickerType;
      }
    }
  }
}



void Graph::checkZoomLevelForNewPlot(Plot *plot)
{
  if(size() < 2)
  {
    queueRedraw();
    return;
  }
  if(isZoomedOutToTop())
  {
    checkScales();
    return;
  }

  // Figure out the scales (zoom levels) for this new plot.
  if(isSameScale)
  {
    // fix the scales later.
    checkScalesQueued = true;
    queueRedraw();

    // Copy all the ZoomLevels
    Plot *p = *begin();
    ZoomLevel *z = p->firstZoomLevel;
    plot->currentZoomLevel->copy(z);

    for(z = z->next; z != p->lastZoomLevel->next; z = z->next)
    {
      // make a new zoomLevel
      plot->zoomIn(0,0,10,10);
      // set this zoomLevel to match.
      plot->currentZoomLevel->copy(z);
    }

    // Find the Current ZoomLevel
    plot->zoomOutToTop();
    z = p->firstZoomLevel;
    for(;z != p->currentZoomLevel; z = z->next)
    {
      plot->zoomBackIn();
    }
  }
  else
  {
    // If we have any zooming than now we will have Plots with mixed
    // levels of zoom and there's nothing wrong with that.  There on
    // different scales anyway.
    
    queueRedraw();
  }
}

void Graph::remove(Plot *plot)
{
  std::list<Plot *>::remove(plot);

  // we let the plot get deleted by other things.
  if(isZoomedOutToTop())
    checkScales();
  else
  {
    if(size() < 2 && (autoSameScale || sameScale))
      isSameScale = true;
    checkScalesQueued = true;
    queueRedraw();
  }

  setPickerType();
  m_signal_removedPlot.emit(this, plot);
}

void Graph::checkScales(void)
{
  bool old_isSameScale = isSameScale;
  
  //opSpew << "void Graph::checkScales(void)" << std::endl;
  if(size() > 1 && sameScale)
  {
    value_t xmax=-MAXVALUE, xmin=MAXVALUE, ymax=-MAXVALUE, ymin=MAXVALUE;

    // get the max and min values from all plots.
    std::list<Plot *>::const_iterator p;
    for(p=begin();p != end(); p++)
    {
      if((*p)->xmax > xmax)
        xmax = (*p)->xmax;
      if((*p)->xmin < xmin)
        xmin = (*p)->xmin;
      
      if((*p)->ymax > ymax)
        ymax = (*p)->ymax;
      if((*p)->ymin < ymin)
        ymin = (*p)->ymin;
    }
    
    if(xmax == xmin)
    {
      xmax += (value_t) 0.5;
      xmin -= (value_t) 0.5;
    }
    if(ymax == ymin)
    {
      ymax += (value_t) 0.5;
      ymin -= (value_t) 0.5;
    }
    
    for(p = begin();p != end(); p++)
    {
      (*p)->resetZoomLevels(xmax, xmin, ymax, ymin);
    }
    isSameScale = true;
  }
  else if(size() > 1 && autoSameScale)
  {
     value_t xmax=-MAXVALUE, xmin=MAXVALUE, ymax=-MAXVALUE, ymin=MAXVALUE;
     value_t xminspan=MAXVALUE, yminspan=MAXVALUE;

    // get the max and min values and xmaxspan, xminspan, ymaxspan,
    // yminspan from all plots.
    std::list<Plot *>::const_iterator p;
    
    for(p=begin();p != end(); p++)
    {
      if((*p)->xmax > xmax)
        xmax = (*p)->xmax;
      if((*p)->xmin < xmin)
        xmin = (*p)->xmin;
      
      if((*p)->ymax > ymax)
        ymax = (*p)->ymax;
      if((*p)->ymin < ymin)
        ymin = (*p)->ymin;

      if((*p)->xmax != (*p)->xmin && (*p)->xmax - (*p)->xmin < xminspan)
          xminspan = (*p)->xmax - (*p)->xmin;

      if((*p)->ymax != (*p)->ymin && (*p)->ymax - (*p)->ymin < yminspan)
          yminspan = (*p)->ymax - (*p)->ymin;
    }

    if((xminspan == MAXVALUE || (xmax - xmin)/xminspan < ((value_t) 10.0)) &&
       (yminspan == MAXVALUE || (ymax - ymin)/yminspan < ((value_t) 10.0))  )
    {
      if(xmax == xmin)
      {
        xmax += (value_t) 0.5;
        xmin -= (value_t) 0.5;
      }
      if(ymax == ymin)
      {
        ymax += (value_t) 0.5;
        ymin -= (value_t) 0.5;
      }
    
      for(p = begin();p != end(); p++)
      {
        (*p)->resetZoomLevels(xmax, xmin, ymax, ymin);
      }
      isSameScale = true;
    }
    else
    {
      std::list<Plot *>::const_iterator p;
      for(p = begin();p != end(); p++)
      {
        (*p)->resetZoomLevels();
      }
      isSameScale = false;
    }
  }
  else if(size() == 1)
  {
    (*begin())->resetZoomLevels();
    isSameScale = true;
  }
  else if(size() > 1)
  {
    std::list<Plot *>::const_iterator p;
    for(p = begin();p != end(); p++)
    {
      (*p)->resetZoomLevels();
    }
    isSameScale = false;
  }

  if(isSameScale != old_isSameScale)
    // Tell the world the news.
    m_signal_changedSameScale.emit(this);
  
  checkScalesQueued = false;
  queueRedraw();
}


void Graph::savePNG(const char *filename)
{
  Glib::RefPtr<Gdk::Window> window = get_window();
  Glib::RefPtr<Gdk::GC> _gc = Gdk::GC::create(window);
  int w, h, depth = window->get_depth();
  window->get_size(w,h);
  
  Glib::RefPtr<Gdk::Drawable> pixmap =
    Gdk::Pixmap::create(window, w, h, depth);

  _gc->set_foreground(backgroundColor);
  pixmap->draw_rectangle(_gc, true, 0, 0, w, h);
  
  draw(pixmap, _gc);
  
  Glib::RefPtr<Gdk::Colormap> colormap = get_default_colormap();
  
  Glib::RefPtr<Gdk::Pixbuf> pixbuf =
    Gdk::Pixbuf::create(pixmap, colormap, 0, 0, 0, 0, w, h);

  try
  {
    pixbuf->save(filename, "png");
  }
  catch(Glib::FileError e)
  {
    if(!opSilent)
      opSpew << "Quickplot WARNING: Can't save PNG file: "
             << filename << " Glib::FileError::code()="
             << e.code() << "." << std::endl;
  }
  catch(Gdk::PixbufError e)
  {
    if(!opSilent)
      opSpew << "Quickplot WARNING: Can't save PNG file: "
             << filename << " Gdk::PixbufError::code()="
             << e.code() << "." << std::endl; 
  }
}

void Graph::queueRedraw(void)
{
  // ignore additional redraw requests until after
  // Graph::on_expose_event() and don't do this if this was called
  // while in Graph::~Graph().  Like when Plot::~Plot() calls this.
  if(isDeleting) 
    return;
  
  GdkRectangle rec =
    {
      0, 0, get_width(), get_height()
    };
  
  gdk_window_invalidate_rect(get_window()->gobj(), &rec, true);
  //gdk_window_process_updates(get_window()->gobj(), true);

}

void Graph::draw(Glib::RefPtr<Gdk::Drawable> _win,
                      Glib::RefPtr<Gdk::GC> _gc)
{
  if(size() > 0) // there is one plot or more.
  {
    std::list<Plot *>::const_iterator plot = begin();
    if(showAutoGrid && isSameScale)
    {
      drawAutoGrid(_win, _gc);
    }
    
    for(; plot != end(); plot++)
    {
      (*plot)->draw(_win, _gc);
    }
  }
}

// This seems to be like QT's qtWidget::draw().
bool Graph::on_expose_event(GdkEventExpose *expose)
{
  if(win.is_null())
  {
    win = get_window();
    gc = Gdk::GC::create(win);
    inverted_gc = Gdk::GC::create(win);
    //inverted_gc->set_function(Gdk::INVERT);
    inverted_gc->set_function(Gdk::XOR);
    
    Glib::RefPtr<Gdk::Colormap> colormap = get_default_colormap();
    
    colormap->alloc_color(backgroundColor);
    win->set_background(backgroundColor);
    
    colormap->alloc_color(gridColor);

    Gdk::Color black;
    black.set_rgb(0,0,0);
    colormap->alloc_color(black);
    inverted_gc->set_foreground(gridColor);
  }

  if(currentMainWindow->statusBar.is_visible())
  {
    currentMainWindow->statusBar.positionEntry.set_text("");
  }

  
  win->clear();
  
  draw(win, gc);

    // This is just for zoom boxes and mouse value pick lines.
  if(lastPickerType != NONE &&
     old_width == get_width() &&
     old_height == get_height() )
  {
    // Need to draw some lines.

    if(lastPickerType == OFF_PLOT)
    {
      if(xpick != SMALL_INT)
      {
        // draw the cross.
        win->draw_line(inverted_gc, 0, ypick, get_width(), ypick);
        win->draw_line(inverted_gc, xpick, 0, xpick, get_height());
      }
    }
    else // lastPickerType == INTERPOLATED || lastPickerType == NONINTERPOLATED
    {
      if(xpick != SMALL_INT)
      {
        // draw the line.
        win->draw_line(inverted_gc, xpick, 0, xpick, get_height());
      }
    }
  }
  else if(old_width != get_width() ||
          old_height != get_height())
  {
    lastPickerType = NONE;
    old_width = get_width();
    old_height = get_height();
  }
  
  return true;
}



bool Graph::isZoomedOutToTop(void)
{
  if(size() < 1) return true;
  std::list<Plot *>::const_iterator plot;
  for(plot=begin(); plot != end(); plot++)
  {
    if(!((*plot)->isZoomedOutToTop()))
      break;
  }
  return (plot == end());
}


void Graph::zoomToTop(void)
{
  if(isZoomedOutToTop()) return; // already there.
  
  std::list<Plot *>::const_iterator plot;
  for(plot=begin(); plot != end(); plot++)
    (*plot)->zoomOutToTop();

  if(checkScalesQueued == true)
    checkScales();
  else
    queueRedraw();
}

void Graph::zoomOut(void)
{
  if(isZoomedOutToTop()) return; // already zoomed to top.

  std::list<Plot *>::const_iterator plot;
  for(plot=begin(); plot != end(); plot++)
  {
    (*plot)->zoomOut();
  }

  if(isZoomedOutToTop() && checkScalesQueued == true)
    checkScales();
  else
    queueRedraw();
}

void Graph::zoomIn(int x, int y, int w, int h)
{
  if(size() < 1) return; // no plots in list

  std::list<Plot *>::const_iterator plot;
  for(plot=begin(); plot != end(); plot++)
    (*plot)->zoomIn(x,y,w,h);
  
  queueRedraw();
}


// These following 3 methods will act on a button that is pressed when
// no other buttons are pressed.  So these will ignore
// multi-button-presses.
bool Graph::on_button_press_event(GdkEventButton *event)
{
  //opSpew << "Graph::on_button_press_event() button=" << event->button
  //       << " x=" << event->x << " y=" << event->y << std::endl;


  if(lastPickerType != NONE && event->button != opZoomButton)
  {
    // Need to undraw some lines.

    if(lastPickerType == OFF_PLOT)
    {
      if(xpick != SMALL_INT)
      {
        // undraw the cross.
        win->draw_line(inverted_gc, 0, ypick, get_width(), ypick);
        win->draw_line(inverted_gc, xpick, 0, xpick, get_height());
        xpick = SMALL_INT;
      }
    }
    else // lastPickerType == INTERPOLATED || lastPickerType == NONINTERPOLATED
    {
      if(xpick != SMALL_INT)
      {
        // undraw the line.
        win->draw_line(inverted_gc, xpick, 0, xpick, get_height());
        xpick = SMALL_INT;
      }
    }
    lastPickerType = NONE;
  }
  
  // Ignore more button presses until buttonPressed is unset.
  if(buttonPressed) return true;

  if(event->button == opPickButton)
  {
    buttonPressed = opPickButton;
    if(size() > 0)
    {
      currentMainWindow->showPlotLister();

      std::list<Plot *>::const_iterator plot = begin();
      if(pickerType == OFF_PLOT)
      {
        xpick = (int) event->x;
        ypick = (int) event->y;
        win->draw_line(inverted_gc, 0, ypick, get_width(), ypick);
        win->draw_line(inverted_gc, xpick, 0, xpick, get_height());

        for(plot=begin(); plot != end(); plot++)
          (*plot)->emitDisplayValues(xpick, ypick);
        
        lastPickerType = OFF_PLOT;
      }
      else if(pickerType == INTERPOLATED)
      {
        xpick = (int) event->x;
        win->draw_line(inverted_gc, xpick, 0, xpick, get_height());
        for(plot=begin(); plot != end(); plot++)
          (*plot)->emitInterpolatedDisplayValues(xpick);

        lastPickerType = INTERPOLATED;
      }
      else // pickerType == NONINTERPOLATED
      {
        for(plot=begin(); plot != end(); plot++)
          xpick = (*plot)->emitDisplayValues((int) event->x);
        win->draw_line(inverted_gc, xpick, 0, xpick, get_height());

        lastPickerType = NONINTERPOLATED;
      }
    }
  }
  else if(event->button == opZoomButton)
  {
    xPress = (int) event->x;
    yPress = (int) event->y;
    buttonPressed = opZoomButton;
    xmouse = SMALL_INT;
  }
  
  return true;
}

bool Graph::on_motion_notify_event(GdkEventMotion *event)
{
  //opSpew << "Graph::on_motion_notify_event()" << std::endl;

  if(size() > 0 && isSameScale && currentMainWindow->statusBar.is_visible())
  {
    setStatusXYValues(event->x, event->y);
  }
  else if(currentMainWindow->statusBar.is_visible())
  {
    currentMainWindow->statusBar.positionEntry.set_text("");
  }
  
  if(!buttonPressed) return true;

  
  if(buttonPressed == opPickButton)
  {
    if(size() > 0)
    {
      std::list<Plot *>::const_iterator plot = begin();

      if(pickerType == OFF_PLOT)
      {
        if(xpick != SMALL_INT)
        {
          // undraw the cross.
          win->draw_line(inverted_gc, 0, ypick, get_width(), ypick);
          win->draw_line(inverted_gc, xpick, 0, xpick, get_height());
        }

        xpick = (int) event->x;
        ypick = (int) event->y;
        win->draw_line(inverted_gc, 0, ypick, get_width(), ypick);
        win->draw_line(inverted_gc, xpick, 0, xpick, get_height());
        
        for(plot=begin(); plot != end(); plot++)
          (*plot)->emitDisplayValues(xpick, ypick);
      }
      else if(pickerType == INTERPOLATED)
      {
        if(xpick != SMALL_INT)
        {
          // undraw the line.
          win->draw_line(inverted_gc, xpick, 0, xpick, get_height());
        }
        xpick = (int) event->x;
        win->draw_line(inverted_gc, xpick, 0, xpick, get_height());
        for(plot=begin(); plot != end(); plot++)
          (*plot)->emitInterpolatedDisplayValues(xpick);
      }
      else // pickerType == NONINTERPOLATED
      {
        if(xpick != SMALL_INT)
        {
          // undraw the line.
          win->draw_line(inverted_gc, xpick, 0, xpick, get_height());
        }
        for(plot=begin(); plot != end(); plot++)
          xpick = (*plot)->emitDisplayValues((int) event->x);
        win->draw_line(inverted_gc, xpick, 0, xpick, get_height());
      }
    }
  }
  else if(buttonPressed == opZoomButton)
  {
    if(xmouse != SMALL_INT) // undraw the past zoom box.
    {
      win->draw_line(inverted_gc, xPress, yPress, xmouse, yPress);
      win->draw_line(inverted_gc, xmouse, yPress, xmouse, ymouse);
      win->draw_line(inverted_gc, xmouse, ymouse, xPress, ymouse);
      win->draw_line(inverted_gc, xPress, ymouse, xPress, yPress);
      xmouse = SMALL_INT;
    }

    if(xPress != (int) event->x && yPress != (int) event->y)
    {
      // draw the zoom box
      xmouse = (int) event->x;
      ymouse = (int) event->y;
      win->draw_line(inverted_gc, xPress, yPress, xmouse, yPress);
      win->draw_line(inverted_gc, xmouse, yPress, xmouse, ymouse);
      win->draw_line(inverted_gc, xmouse, ymouse, xPress, ymouse);
      win->draw_line(inverted_gc, xPress, ymouse, xPress, yPress);
    }
  }
  
  return true;
}

bool Graph::on_button_release_event(GdkEventButton *event)
{
  //opSpew << "Graph::on_button_release_event() button=" << event->button
  //       << " x=" << event->x << " y=" << event->y << std::endl;

  
  // Ignore other button releases.
  if(buttonPressed != event->button) return true;


  if(buttonPressed == opPickButton)
  {
    if(size() > 0)
    {
      std::list<Plot *>::const_iterator plot = begin();

      if(pickerType == OFF_PLOT)
      {
        if(xpick != SMALL_INT)
        {
          // undraw the cross.
          win->draw_line(inverted_gc, 0, ypick, get_width(), ypick);
          win->draw_line(inverted_gc, xpick, 0, xpick, get_height());
        }

        xpick = (int) event->x;
        ypick = (int) event->y;
        win->draw_line(inverted_gc, 0, ypick, get_width(), ypick);
        win->draw_line(inverted_gc, xpick, 0, xpick, get_height());
        
        for(plot=begin(); plot != end(); plot++)
          (*plot)->emitDisplayValues(xpick, ypick);
      }
      else if(pickerType == INTERPOLATED)
      {
        if(xpick != SMALL_INT)
        {
          // undraw the line.
          win->draw_line(inverted_gc, xpick, 0, xpick, get_height());
        }
        xpick = (int) event->x;
        win->draw_line(inverted_gc, xpick, 0, xpick, get_height());
        for(plot=begin(); plot != end(); plot++)
          (*plot)->emitInterpolatedDisplayValues(xpick);
      }
      else // pickerType == NONINTERPOLATED
      {
        if(xpick != SMALL_INT)
        {
          // undraw the line.
          win->draw_line(inverted_gc, xpick, 0, xpick, get_height());
        }
        for(plot=begin(); plot != end(); plot++)
          xpick = (*plot)->emitDisplayValues((int) event->x);
        win->draw_line(inverted_gc, xpick, 0, xpick, get_height());
      }
    }
  }
  else if(buttonPressed == opZoomButton)
  {
    if(xmouse != SMALL_INT) // undraw the past zoom box.
    {
      win->draw_line(inverted_gc, xPress, yPress, xmouse, yPress);
      win->draw_line(inverted_gc, xmouse, yPress, xmouse, ymouse);
      win->draw_line(inverted_gc, xmouse, ymouse, xPress, ymouse);
      win->draw_line(inverted_gc, xPress, ymouse, xPress, yPress);
      xmouse = SMALL_INT;
    }

    // zoom box is a line or a point.
    if(xPress == (int) event->x || yPress == (int) event->y)
    {
    }

    // zoom box is out side of one side of the Graph.
    else if(
      (
        (get_width() <= (int) event->x || 0 > (int) event->x)
        &&
        (get_height() > (int) event->y && 0 <= (int) event->y)
        )
       ||
       (
         (get_height() <= (int) event->y || 0 > (int) event->y)
         &&
         (get_width() > (int) event->x && 0 <= (int) event->x)
         )
      )
    {
      zoomOut();
    }
    else if((get_width() <= (int) event->x || 0 > (int) event->x)
            &&
            (get_height() <= (int) event->y || 0 > (int) event->y)
            )
    {
      zoomToTop();
    }
    else
    {
      // get the zoom box parameters x, y, w, and h.
      int x = (xPress < (int) event->x)? xPress : (int) event->x;
      int y = (yPress < (int) event->y)? yPress : (int) event->y;
      int w = ABSVAL(xPress - (int) event->x);
      int h = ABSVAL(yPress - (int) event->y);
      
      zoomIn(x,y,w,h);
    }

    if(lastPickerType != NONE)
    {
      // Need to undraw some lines.
      
      if(lastPickerType == OFF_PLOT)
      {
        if(xpick != SMALL_INT)
        {
          // undraw the cross.
          win->draw_line(inverted_gc, 0, ypick, get_width(), ypick);
          win->draw_line(inverted_gc, xpick, 0, xpick, get_height());
          xpick = SMALL_INT;
        }
      }
      else // lastPickerType == INTERPOLATED || lastPickerType == NONINTERPOLATED
      {
        if(xpick != SMALL_INT)
        {
          // undraw the line.
          win->draw_line(inverted_gc, xpick, 0, xpick, get_height());
          xpick = SMALL_INT;
        }
      }
      lastPickerType = NONE;
    }
  }
  
  if(size() > 0 && isSameScale && currentMainWindow->statusBar.is_visible())
  {
    // need to show the x,y values with this new scale.
    setStatusXYValues(event->x, event->y);
  }
  else if(currentMainWindow->statusBar.is_visible())
  {
    currentMainWindow->statusBar.positionEntry.set_text("");
  }

  
  buttonPressed = 0;
  return true;
}

void Graph::setStatusXYValues(gdouble x, gdouble y)
{
  ZoomLevel *z = front()->currentZoomLevel;
  
  // x
  value_t X = (x - z->shiftX)/z->scaleX;
  value_t Y = (y - z->shiftY)/z->scaleY;
  char s[48];
  snprintf(s,48,FORMAT"  "FORMAT, X, Y);
  currentMainWindow->statusBar.positionEntry.set_text(s);
}

void Graph::createDefaultPlots(Source *source)
{
  if(!source) return; // this shouldn't happen.
  
  int numPlots = source->size() - 1;
  if(numPlots > opMaxNumDefaultPlots)
    numPlots = opMaxNumDefaultPlots;
  
  if(numPlots > 0)
  {
    int i = 0;
    std::list<Field *>::const_iterator field = source->begin();
    Field *x = *field;
    for(field++;field != source->end() && i < numPlots; field++)
    {
      createPlot(x, *field);
      i++;
    }
  }
}

void Graph::setShowLines(bool do_show)
{
  showLines = do_show;
  
  // set the current plots
  std::list<Plot *>::const_iterator plot = begin();
  for(;plot != end(); plot++)
  {
    (*plot)->setShowLines(showLines);
  }
  queueRedraw();
}

void Graph::setShowPoints(bool do_show)
{
  showPoints = do_show;
  
  // set the current plots
  std::list<Plot *>::const_iterator plot = begin();
  for(;plot != end(); plot++)
  {
    (*plot)->setShowPoints(showPoints);
  }
  queueRedraw();
}

void Graph::setLineWidth(int width, bool oldPlotsToo)
{
  lineWidth = width;

  if(oldPlotsToo)
  {
    // set the current plots
    std::list<Plot *>::const_iterator plot = begin();
    for(;plot != end(); plot++)
    {
      (*plot)->setLineWidth(lineWidth);
    }
  }
  queueRedraw();
}

void Graph::setPointSize(int size,  bool oldPlotsToo)
{
  pointSize = size;

  if(oldPlotsToo)
  {
    // set the current plots
    std::list<Plot *>::const_iterator plot = begin();
    for(;plot != end(); plot++)
    {
      (*plot)->setPointSize(pointSize);
    }
  }
  queueRedraw();
}

void Graph::setBackgroundColor(const Gdk::Color& color)
{
  backgroundColor = color;

  if(!(win.is_null()))
  {
    get_default_colormap()->alloc_color(backgroundColor);
    win = get_window();
    win->set_background(backgroundColor);
  }

  m_signal_backgroundColorChanged.emit();
}

void Graph::setGridColor(const Gdk::Color& color)
{
  gridColor = color;
  
  if(!(win.is_null()))
  {
    get_default_colormap()->alloc_color(gridColor);
  }
}




GraphTab::GraphTab(int count, MainWindow *mainWindow_in, Graph *graph_in) :
  closeImage(Stock::CLOSE, ICON_SIZE_MENU)
{
  graph = graph_in;
  mainWindow = mainWindow_in;
  
  set_spacing(6);
  
  char s[16];
  snprintf(s, 16, "Graph %d", count);
  label.set_text(s);
  add(label);
  add(removeButton);

  removeButton.set_size_request(15,15);
  removeButton.add(closeImage);

  removeButton.signal_pressed().
    connect(SigC::slot(*this, &GraphTab::on_close));
  
  closeImage.show();
  label.show();
  removeButton.show();
}

// Checks size and that it's not the same as another in this main window.
void GraphTab::setText(const char *str)
{
  if(!str || !str[0]) return;
  
  const size_t maxSize = 20;
  char s[maxSize];

  // truncated copy.
  snprintf(s, maxSize, "%s", str);
  size_t end = strlen(s);

  int n = mainWindow->graphsNotebook.get_n_pages();

  int j;
  for(j=2;j<1000; j++)
  {
    int i;
    for(i=0;i<n;i++)
    {
      GraphTab *tab = dynamic_cast<GraphTab *>
        (mainWindow->graphsNotebook
         .get_tab_label(*(mainWindow->graphsNotebook.get_nth_page(i))));
      //printf("comparing \"%s\" \"%s\"\n",tab->label.get_text().c_str(), s);
      if(tab && !strcmp(tab->label.get_text().c_str(), s) && tab != this)
        break;
    }
    
    if(i != n) // We have a matching label
    {
      // Put a different number at the end of the label.

      if(strlen(s) > end)
	s[end] = '\0';

      // clear the end of the string.
      if(j<10)
        s[maxSize-5] = '\0';
      else if(j<100)
        s[maxSize-6] = '\0';
      else
        s[maxSize-7] = '\0';
      
      sprintf(&s[strlen(s)], " (%d)", j);
    }
    else // no matching labels
      break;
  }
 
  label.set_text(s);
}


extern "C"
{
  static gboolean GraphTab_deleteLater(gpointer data)
  {
    gtk_idle_remove_by_data(data);
    GraphTab::DeleteLater *d = (GraphTab::DeleteLater *) data;
    d->graphTab->mainWindow->removeGraphTab(d->graphTab->graph);
    return ((gboolean) 0);
  }
}


void GraphTab::on_close(void)
{
  deleteLater.graphTab = this;
  gtk_idle_add(GraphTab_deleteLater, &deleteLater);
}



SigC::Signal1<void, Graph *> Graph::signal_addedPlot(void)
{
  return m_signal_addedPlot;
}

SigC::Signal2<void, Graph *, Plot *> Graph::signal_removedPlot(void)
{
  return m_signal_removedPlot;
}

SigC::Signal1<void, Graph *> Graph::m_signal_addedPlot;

SigC::Signal2<void, Graph *, Plot *> Graph::m_signal_removedPlot;



SigC::Signal1<void, Graph *> Graph::signal_changedSameScale(void)
{
  return m_signal_changedSameScale;
}

SigC::Signal1<void, Graph *> Graph::m_signal_changedSameScale;


SigC::Signal0<void> Graph::signal_backgroundColorChanged(void)
{
  return m_signal_backgroundColorChanged;
}

