/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */
#include "config.h"

#ifdef Darwin
# include <limits.h>
# include <float.h>
#else
# include <values.h>
#endif

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

#include "Field.h"
#include "LinearField.h"

#include "Plot.h"
#include "ColorGen.h"
#include "Graph.h"
#include "Globel.h"


// SPACE is the fraction of blank space on graphing area for the first
// auto zoom level.  The blank space is split evenly between the top
// and bottom and on the right and left side.  So for example the size
// of the blank space on the top and bottom is graph_height*SPACE/2
// (pixels) and on the right and left side it's graph_width*SPACE/2
// (pixels).
#define SPACE  ((value_t) 0.06) 
#define FILL   (((value_t) 1.0) - SPACE)


void Plot::init(void)
{
  graph->push_back(this);
  // Get what plot number is this in the Graph.
  plotCount = (int) graph->size(); // graph list size.

  pickerXDequeuer = NULL;
  
  // Set the default label.
  {
    char *format = "%s (%s) VS %s (%s)";
    size_t len = strlen(format);
    len += strlen(_y->getLabel()) + 1;
    len += strlen(_y->getName()) + 1;
    len += strlen(_x->getLabel()) + 1;
    len += strlen(_x->getName()) + 1;
    label = (char *) malloc(sizeof(char)*len);
    sprintf(label, format,
            _y->getLabel(), _y->getName(),
            _x->getLabel(), _x->getName());
  }
  
  setWidthHeightDepParmeters();
  
  { // Find the number of values to plot.

    // The Linear field has an undetermined number of values.
    
    LinearField *lx = dynamic_cast<LinearField *>(_x);
    LinearField *ly = dynamic_cast<LinearField *>(_y);
    
    if(!lx && !ly)
    {
      numberOfValues =
        (_x->numberOfValues() < _y->numberOfValues())?
        _x->numberOfValues():_y->numberOfValues();
    }
    else if(lx && !ly)
    {
      numberOfValues = _y->numberOfValues();
    }
    else if(!lx && ly)
    {
      numberOfValues = _x->numberOfValues();
    }
    else // (lx && ly)
    {
      numberOfValues = 10000;
    }
  }
  
  xmax = _x->max(numberOfValues-1);
  xmin = _x->min(numberOfValues-1);
  ymax = _y->max(numberOfValues-1);
  ymin = _y->min(numberOfValues-1);

#if 0
  opSpew << " xmax=" << xmax << " xmin=" << xmin <<
    " ymax=" << ymax << " ymin=" << ymin << std::endl;
#endif

  firstZoomLevel = currentZoomLevel = lastZoomLevel = NULL;

  firstZoomLevel = currentZoomLevel = lastZoomLevel =
    new ZoomLevel(this, width, height, xmax, xmin, ymax, ymin);
  
  // need a line color and a point color.
  gushort lr, lg, lb, pr, pg, pb;
  // There's a ColorGen object in the graph that asigns the default
  // plot colors.
  graph->colorGen.getColors(lr, lg, lb, pr, pg, pb);

  // I don't know how this colormap thing works, but this seems to
  // work.
  Glib::RefPtr<Gdk::Colormap> colormap = graph->get_default_colormap();
  lineColor.set_red(lr);
  lineColor.set_green(lg);
  lineColor.set_blue(lb);
  colormap->alloc_color(lineColor);
  
  pointColor.set_red(pr);
  pointColor.set_green(pg);
  pointColor.set_blue(pb);
  colormap->alloc_color(pointColor);

  if(!opUserSetLineOrPointOption && numberOfValues >= 1000000)
  {
    showLines  = false;
    showPoints = true;
    lineWidth   = 0;
    pointSize  = 0;
  }
  else
  {
    // defaults for plots from graph
    showLines  = graph->showLines;
    showPoints = graph->showPoints;
    lineWidth   = graph->lineWidth;
    pointSize  = graph->pointSize;
  }
  
  pointSizeDiv2 = pointSize/2;

  widthPlus = (value_t) (width + lineWidth + 1);
  heightPlus = (value_t) (height + lineWidth + 1);
  lineWidthPlus1 = (value_t) (lineWidth + 1);

  drawLineFunc = &Plot::drawLine0;
  drawPointFunc = &Plot::drawPoint0;

  xpick = ypick = MAXVALUE;
}

// Construct a copy of plot
Plot::Plot(Graph *graph_in, Plot *plot):
  graph(graph_in),
  _x(plot->x()), _y(plot->y())
{
  init();

  // copy parameters.
  lineColor = plot->lineColor;
  pointColor = plot->pointColor;
  
  showLines = plot->showLines;
  showPoints = plot->showPoints;
  lineWidth   = plot->lineWidth;
  pointSize  = plot->pointSize;
  
  pointSizeDiv2 = pointSize/2;

  drawLineFunc = plot->drawLineFunc;
  drawPointFunc = plot->drawPointFunc;

  
  setWidthHeightDepParmeters();

  // Copy all the ZoomLevels
  ZoomLevel *z = plot->firstZoomLevel;
  currentZoomLevel->copy(z);

  for(z = z->next; z != plot->lastZoomLevel->next; z = z->next)
  {
    // make a new zoomLevel
    zoomIn(0,0,10,10);
    // set this zoomLevel to match.
    currentZoomLevel->copy(z);
  }

  // Find the Current ZoomLevel
  zoomOutToTop();
  z = plot->firstZoomLevel;
  for(;z != plot->currentZoomLevel; z = z->next)
  {
    zoomBackIn();
  }

  resize();
}

Plot::Plot(Graph *graph_in, Field *X, Field *Y):
  graph(graph_in),
  _x(X), _y(Y)
{
  init();
}

void Plot::setWidthHeightDepParmeters(void)
{
  width = graph->get_width();
  height = graph->get_height();

  widthPlus = (value_t) (width + lineWidth + 1);
  heightPlus = (value_t) (height + lineWidth + 1);
  lineWidthPlus1 = (value_t) (lineWidth + 1);
}

// This is for when the Plots in a Graph are all set to have the
// same scales.
void Plot::resetZoomLevels(value_t xmax_in, value_t xmin_in,
                           value_t ymax_in, value_t ymin_in)
{
  // Zoom all the way out and remove the previous zoom levels.
  lastZoomLevel = currentZoomLevel = firstZoomLevel;
  
  currentZoomLevel->reset(width, height, xmax_in, xmin_in, ymax_in, ymin_in);
}

// This is for when the Plots in a Graph are all set to have the
// different scales.
void Plot::resetZoomLevels(void)
{
  // Zoom all the way out and remove the previous zoom levels.
  lastZoomLevel = currentZoomLevel = firstZoomLevel;
  
  currentZoomLevel->reset(width, height, xmax, xmin, ymax, ymin);
}



// These are the default 0 draw methods.
void Plot::drawPoint0(int X, int Y)
{
  if(pointSize <= 1)
    win->draw_point(gc, X, Y);
  else
  {
    win->draw_rectangle(gc, true,
                        X-pointSizeDiv2, Y-pointSizeDiv2,
                        pointSize, pointSize);
  }
}

void Plot::drawLine0(int fromX, int fromY, int toX, int toY)
{
  win->draw_line(gc, fromX, fromY, toX, toY);
}


Plot::~Plot()
{
  graph->remove(this);
  graph->queueRedraw();

  // delete all the zoom levels.
  while(firstZoomLevel)
    delete firstZoomLevel;

  if(pickerXDequeuer)
  {
    _x->destroyDequeuer(pickerXDequeuer);
    _y->destroyDequeuer(pickerYDequeuer);
  }
}

// Cull and Draw
void Plot::cullDrawPoint(value_t X, value_t Y)
{
  if(X > -pointSize && X < (value_t) (width+pointSize) &&
     Y > -pointSize && Y < (value_t) (height+pointSize))
  {
    (this->*drawPointFunc)((int) X, (int) Y);
  }
}

void Plot::drawPoints(void)
{
  gc->set_foreground(pointColor);
  
  value_t scaleX = currentZoomLevel->scaleX;
  value_t shiftX = currentZoomLevel->shiftX;
  
  value_t scaleY = currentZoomLevel->scaleY;
  value_t shiftY = currentZoomLevel->shiftY;

  if(_x != _y)
  {
    _x->rewind();
    _y->rewind();
    
    count_t i;
    for(i=0;i<numberOfValues;i++)
    {
      value_t X = _x->read()*scaleX + shiftX;
      value_t Y = _y->read()*scaleY + shiftY;
      
      cullDrawPoint(X, Y);
    }
  }
  else // _x == _y   the X and Y Fields are the same
    // This is not very interesting, but they asked for it.
  {
    _x->rewind();
    
    count_t i;
    for(i=0;i<numberOfValues;i++)
    {
      value_t X = _x->read();
      
      cullDrawPoint(X*scaleX + shiftX, X*scaleY + shiftY);
    }
  }
}

void Plot::drawLines(void)
{
  gc->set_line_attributes (lineWidth, Gdk::LINE_SOLID,
                           Gdk::CAP_ROUND, Gdk::JOIN_ROUND);
  gc->set_foreground(lineColor);
  
  value_t scaleX = currentZoomLevel->scaleX;
  value_t shiftX = currentZoomLevel->shiftX;
  
  value_t scaleY = currentZoomLevel->scaleY;
  value_t shiftY = currentZoomLevel->shiftY;

  if(_x != _y)
  {
    _x->rewind();
    _y->rewind();
    
    value_t fromX = _x->read()*scaleX + shiftX;
    value_t fromY = _y->read()*scaleY + shiftY;
    
    count_t i;
    for(i=0;i<numberOfValues-1;i++)
    {
      value_t toX = _x->read()*scaleX + shiftX;
      value_t toY = _y->read()*scaleY + shiftY;
      
      cullDrawLine(fromX, fromY, toX, toY);
      
      fromX = toX;
      fromY = toY;
    }
  }
  else // _x == _y   the X and Y Fields are the same
    // This is not very interesting, but they asked for it.
  {
    _x->rewind();
    
    value_t X = _x->read();
    
    value_t fromX = X*scaleX + shiftX;
    value_t fromY = X*scaleY + shiftY;
    
    count_t i;
    for(i=0;i<numberOfValues-1;i++)
    {
      value_t X = _x->read();
      
      value_t toX = X*scaleX + shiftX;
      value_t toY = X*scaleY + shiftY;
      
      cullDrawLine(fromX, fromY, toX, toY);
      fromX = toX;
      fromY = toY;
    }
    
  }
}

// must be called before using this class to get scale numbers and/or
// draw to make sure that scales are consistant.  This could be used
// as a signal_resize() slot.
void Plot::preDrawCheckResize(void)
{
  if(width != graph->get_width() || 
     height != graph->get_height())
    resize();
}

void Plot::draw(Glib::RefPtr<Gdk::Drawable> win_in,
                Glib::RefPtr<Gdk::GC> gc_in)
{
  if(!showLines && !showPoints) return;

  preDrawCheckResize();
 
  gc = gc_in;
  win = win_in;
  
  if(showLines)
    drawLines();

  if(showPoints)
    drawPoints();

  gc.clear();
  win.clear();
}

void Plot::resize(void)
{
  ZoomLevel *z = firstZoomLevel;
  ZoomLevel *end = lastZoomLevel->next;

  setWidthHeightDepParmeters();
  
  while(z != end)
  {
    z->reset(width, height);
    z = z->next;
  }
}


// zoOM in with a zoom box.
void Plot::zoomIn(int x, int y, int w, int h)
{
  if(!currentZoomLevel->next)
    currentZoomLevel = lastZoomLevel =
      new ZoomLevel(this, width, height, x, y, w, h);
  else
  {
    currentZoomLevel = lastZoomLevel
      = currentZoomLevel->next;
    currentZoomLevel->reset(width, height, x, y, w, h);
  }
}

void Plot::zoomBackIn(void)
{
  if(currentZoomLevel != lastZoomLevel)
    currentZoomLevel = currentZoomLevel->next;
}

// Zoom out one level.
void Plot::zoomOut(void)
{
  if(currentZoomLevel->prev)
    currentZoomLevel = currentZoomLevel->prev;
}

// Zoom back to the first zoom level, i.e. no zoom.
void Plot::zoomOutToTop(void)
{
  currentZoomLevel = firstZoomLevel;
}

void Plot::setLabel(const char *l)
{
  if(l && l[0])
  {
    if(label)
      free(label);

    label = strdup(l);
  }
}

void Plot::setLineWidth(int width)
{
  lineWidth = width;
  setWidthHeightDepParmeters();
  m_signal_changed.emit();
}

void Plot::setPointSize(int size)
{
  pointSize = size;
  pointSizeDiv2 = pointSize/2;
  m_signal_changed.emit();
}

void Plot::setLineColor(const Gdk::Color &color)
{
  lineColor = color;
  graph->get_default_colormap()->alloc_color(lineColor);
  m_signal_changed.emit();
}

void Plot::setPointColor(const Gdk::Color &color)
{
  pointColor = color;
  graph->get_default_colormap()->alloc_color(pointColor);
  m_signal_changed.emit();
}

// Emitted when the lineColor, pointColor, lineWidth or pointSize
// changes.
SigC::Signal0<void> Plot::signal_changed(void)
{
  return m_signal_changed;
}

// pick a Y value.
int Plot::emitDisplayValues(int X_in)
{
  if(!pickerXDequeuer)
  {
    pickerXDequeuer = _x->makeDequeuer(numberOfValues);
    pickerYDequeuer = _y->makeDequeuer(numberOfValues);
  }    
  
  // It is assumed that the X field is increasing.

  value_t x_val = (X_in - currentZoomLevel->shiftX)/currentZoomLevel->scaleX;
  value_t xUpper, xLower, yUpper, yLower;
  
  xLower = _x->read(pickerXDequeuer);
  yLower = _y->read(pickerYDequeuer);
  xUpper = _x->read(pickerXDequeuer);
  yUpper = _y->read(pickerYDequeuer);

  
  while(xUpper < x_val)
  {
    xLower = xUpper;
    yLower = yUpper;
    xUpper = _x->read(pickerXDequeuer);
    yUpper = _y->read(pickerYDequeuer);
    if(xUpper == xLower)
      break;
  }

  if(xUpper == xLower)
  {
    // We need to push the dequeuer back so we don't read the same
    // value again.
    _x->readBack(pickerXDequeuer);
    _y->readBack(pickerYDequeuer);
  }

  while(xLower > x_val)
  {
    xUpper = xLower;
    yUpper = yLower;
    xLower = _x->readBack(pickerXDequeuer);
    yLower = _y->readBack(pickerYDequeuer);
    if(xUpper == xLower)
      break;
  }

  //opSpew << __LINE__ << " xLower=" << xLower << " xUpper=" << xUpper
  //       << " yLower=" << yLower << " yUpper=" << yUpper
  //      << std::endl;
 

  if(x_val - xLower > xUpper - x_val)
  {
    xpick = xUpper;
    ypick = yUpper;
  }
  else
  {
    xpick = xLower;
    ypick = yLower;
  }

  m_signal_valueDisplay.emit(xpick, ypick);

  return (int) (xpick*currentZoomLevel->scaleX + currentZoomLevel->shiftX);
}

// interpolate a Y value.
void Plot::emitInterpolatedDisplayValues(int X_in)
{
  if(!pickerXDequeuer)
  {
    pickerXDequeuer = _x->makeDequeuer(numberOfValues);
    pickerYDequeuer = _y->makeDequeuer(numberOfValues);
  }    
  
  // It is assumed that the X field is increasing.

  // we will interpolate a Y value.
  value_t x_val = (X_in - currentZoomLevel->shiftX)/currentZoomLevel->scaleX;
  value_t xUpper, xLower, yUpper, yLower;
  
  xLower = _x->read(pickerXDequeuer);
  yLower = _y->read(pickerYDequeuer);
  xUpper = _x->read(pickerXDequeuer);
  yUpper = _y->read(pickerYDequeuer);
  
  
  while(xUpper < x_val)
  {
    xLower = xUpper;
    yLower = yUpper;
    xUpper = _x->read(pickerXDequeuer);
    yUpper = _y->read(pickerYDequeuer);
    if(xUpper == xLower)
      break;
  }

  if(xUpper == xLower)
  {
    // We need to push the dequeuer back so we don't read the same
    // value again.
    _x->readBack(pickerXDequeuer);
    _y->readBack(pickerYDequeuer);
  }

  while(xLower > x_val)
  {
    xUpper = xLower;
    yUpper = yLower;
    xLower = _x->readBack(pickerXDequeuer);
    yLower = _y->readBack(pickerYDequeuer);
    if(xUpper == xLower)
      break;
  }

  //opSpew << __LINE__ << " xLower=" << xLower << " xUpper=" << xUpper
  //       << " yLower=" << yLower << " yUpper=" << yUpper
  //      << std::endl;
 

  if(xUpper != xLower)
  {
    if(xLower <= x_val && xUpper >= x_val)
    {
      xpick = x_val;
      ypick = yLower + (x_val-xLower)*(yUpper-yLower)/(xUpper-xLower);
    }
    else if(xLower > x_val)
    {
      xpick = xLower;
      ypick = yLower;
    }
    else // xUpper < x_val
    {
      xpick = xUpper;
      ypick = yUpper;
    }
  }
  else
  {
    xpick = xUpper;
    ypick = yUpper;
  }

  m_signal_valueDisplay.emit(xpick, ypick);
}

void Plot::emitDisplayValues(int X, int Y)
{
  xpick = (X - currentZoomLevel->shiftX)/currentZoomLevel->scaleX;
  ypick = (Y - currentZoomLevel->shiftY)/currentZoomLevel->scaleY;
  m_signal_valueDisplay.emit(xpick, ypick);           
}

SigC::Signal2<void, value_t, value_t> Plot::signal_valueDisplay(void)
{
  return m_signal_valueDisplay;
}




// For the first Zoom Level with the max and min limits given.
ZoomLevel::ZoomLevel(Plot *plot_in, int width, int height,
                     value_t xmax, value_t xmin,
                     value_t ymax, value_t ymin):
  plot(plot_in)
{
  prev = next = NULL;

  reset(width, height, xmax, xmin, ymax, ymin);
}

// Based on the prev Plot Zoom Level and a zoom box defined by
// (x,y,w,h).
ZoomLevel::ZoomLevel(Plot *plot_in, int width, int height,
                     int x, int y, int w, int h):
  plot(plot_in)
{
  ZoomLevel *z = plot->firstZoomLevel;
  while(z->next)
    z = z->next;
  // z is now the last zoom Level.
  z->next = this;
  prev = z;
  next = NULL;
  
  xMax = (x + w - prev->shiftX)/prev->scaleX;
  xMin = (x - prev->shiftX)/prev->scaleX;
  
  scaleX = width*FILL/(xMax - xMin);
  shiftX = width*SPACE/2 - scaleX * xMin;

  
  yMax = (y - prev->shiftY)/prev->scaleY;
  yMin = (y + h - prev->shiftY)/prev->scaleY;

  scaleY = - height*FILL/(yMax - yMin);
  shiftY =   height*SPACE/2 - scaleY * yMax;
}

void ZoomLevel::reset(int width, int height,
                      int x, int y, int w, int h)
{
  xMax = (x + w - prev->shiftX)/prev->scaleX;
  xMin = (x - prev->shiftX)/prev->scaleX;
  
  scaleX = width*FILL/(xMax - xMin);
  shiftX = width*SPACE/2 - scaleX * xMin;

  
  yMax = (y - prev->shiftY)/prev->scaleY;
  yMin = (y + h - prev->shiftY)/prev->scaleY;

  scaleY = - height*FILL/(yMax - yMin);
  shiftY =   height*SPACE/2 - scaleY * yMax;
}

void ZoomLevel::reset(int width, int height)
{
  scaleX = width*FILL/(xMax - xMin);
  shiftX = width*SPACE/2 - scaleX * xMin;

  scaleY = - height*FILL/(yMax - yMin);
  shiftY =   height*SPACE/2 - scaleY * yMax;
}

// This only works and makes sense for the first zoom level.  This is
// for when the Plots are all set to have the same scales.
void ZoomLevel::reset(int width, int height,
                      value_t xmax, value_t xmin,
                      value_t ymax, value_t ymin)
{
  if(xmax != xmin)
  {
    xMax = xmax;
    xMin = xmin;
  }
  else // xmax == xmin
  {
    if((plot->plotCount)%2) // odd
    {
      xMax = xmax + (value_t) 0.1 + (value_t) 0.3/((value_t)plot->plotCount);
      xMin = xmin - (value_t) 0.1;
    }
    else // even
    {
      xMax = xmax + (value_t) 0.1;
      xMin = xmin - (value_t) 0.1 - (value_t) 0.3/((value_t)(plot->plotCount-1));
    }
  }

  if(ymax != ymin)
  {
    yMax = ymax;
    yMin = ymin;
  }
  else
  {
    if((plot->plotCount)%2) // odd
    {
      yMax = ymax + (value_t) 0.1 + (value_t) 0.3/((value_t)plot->plotCount);
      yMin = ymin - (value_t) 0.1;
    }
    else // even
    {
      yMax = ymax + (value_t) 0.1;
      yMin = ymin - (value_t) 0.1 - (value_t) 0.3/((value_t)(plot->plotCount-1));
    }
  }
  
  scaleX = width*FILL/(xMax - xMin);
  shiftX = width*SPACE/2 - scaleX * xMin;
  
  scaleY = - height*FILL/(yMax - yMin);
  shiftY = height*SPACE/2 - scaleY * yMax;
}

void ZoomLevel::copy(ZoomLevel *z)
{
  scaleX = z->scaleX;
  scaleY = z->scaleY;
  shiftX = z->shiftX;
  shiftY = z->shiftY;

  xMax = z->xMax;
  xMin = z->xMin;
  yMax = z->yMax;
  yMin = z->yMin;
}

ZoomLevel::~ZoomLevel(void)
{
  // Remove this from the double-ly linked list.
  ZoomLevel *z = plot->firstZoomLevel;
  for(; z ; z = z->next)
  {
    if(z == this)
    {
      if(next)
        next->prev = prev;
      if(prev)
        prev->next = next;

      // In case the only zoom levels are in use fix the list in plot.
      if(this == plot->firstZoomLevel)
        plot->firstZoomLevel = next;
      if(this == plot->currentZoomLevel)
        plot->currentZoomLevel = (next) ? next : prev;
      if(this == plot->lastZoomLevel)
        plot->lastZoomLevel = (next) ? next : prev;
      break;
    }
  }
}

