/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */

class ZoomLevel;
class Graph;


class Plot
{
 public:

  Plot(Graph *graph, Field *x, Field *y);
  Plot(Graph *graph, Plot *plot); // copy constructor
  ~Plot(void);

  inline Field *x() { return _x; }
  inline Field *y() { return _y; }

  inline count_t getNumberOfPoints(void) { return numberOfValues; }

  void setLineWidth(int width);
  void setPointSize(int size);
  inline int getLineWidth(void) const { return lineWidth; }
  inline int getPointSize(void) const { return pointSize; }

  inline Gdk::Color &getLineColor(void) { return lineColor; }
  inline Gdk::Color &getPointColor(void) { return pointColor; }
  void setLineColor(const Gdk::Color &color);
  void setPointColor(const Gdk::Color &color);

  // must be called before using this class to get scale numbers
  // and/or draw to make sure that scales are consistant.  This could
  // be used as a signal_resize() slot.
  void preDrawCheckResize(void);
  
  // draw lines and than points
  void draw(Glib::RefPtr<Gdk::Drawable> win,
            Glib::RefPtr<Gdk::GC> gc);

  inline char *getLabel(void) const { return label; }
  void setLabel(const char *label);
  
 
  // This is for when the Plots in a Graph are all set to have the
  // same scales.
  void resetZoomLevels(value_t xmax, value_t xmin,
                       value_t ymax, value_t ymin);
  
  // This is for when the Plots in a Graph are all set to have the
  // different scales.
  void resetZoomLevels(void);

  // zoOM in with a zoom box.
  void zoomIn(int x, int y, int w, int h);
  void zoomBackIn(void);
  
  void zoomOut(void);
  void zoomOutToTop(void);

  inline bool isZoomedOutToTop(void)
    {
      return (currentZoomLevel == firstZoomLevel);
    }
  
  inline void setShowLines(bool do_show=true)
    {
      showLines = do_show;
      m_signal_changed.emit();
    }
  
  inline void setShowPoints(bool do_show=true)
    {
      showPoints = do_show;
      m_signal_changed.emit();
    }
  
  inline bool getShowLines(void) { return showLines; }
  
  inline bool getShowPoints(void) { return showPoints; }
  
  
  // Fix zoomLevels due to a resized graph window.
  void resize(void);


  friend class ZoomLevel;

  
  // The max and min values that are plotted.  They may not be the
  // same as the max and min values in the x and y fields, since all
  // the x and y field values may not be plotted.  Other classes
  // should read (not write) these only.
  value_t xmax, xmin, ymax, ymin;
  
  ZoomLevel *currentZoomLevel;
  
  // Plot contains a list of zoom levels.
  ZoomLevel *firstZoomLevel, *lastZoomLevel;
  
  Graph *graph;

  // Emitted when the lineColor, pointColor, lineWidth, pointSize,
  // showLines and showPoints.
  SigC::Signal0<void> signal_changed(void);

  
  int emitDisplayValues(int X);
  void emitInterpolatedDisplayValues(int X);
  void emitDisplayValues(int X, int Y);

  SigC::Signal2<void, value_t, value_t> signal_valueDisplay(void);
  
  value_t xpick, ypick;

protected:

private:

  bool showPoints, showLines;
  
  SigC::Signal0<void> m_signal_changed;

  SigC::Signal2<void, value_t, value_t> m_signal_valueDisplay;

  Gdk::Color lineColor, pointColor;

  // Cull and Draw.
  void cullDrawLine(value_t fromX, value_t fromY, value_t toX, value_t toY);
  void cullDrawPoint(value_t X, value_t Y);

  // number of points to plot.
  count_t numberOfValues;

  
  Field *_x, *_y;
  char *label;

  int width, height; // Graph width and height for zooming.

  // This saves having to pass win and gc as arguments to the drawing
  // functions.
  Glib::RefPtr<Gdk::Drawable> win;
  Glib::RefPtr<Gdk::GC> gc;


  void setWidthHeightDepParmeters(void);
  

  // point diameter, line width.
  int pointSize, lineWidth, pointSizeDiv2;
  
  // Culling numbers derived from lineWidth
  value_t widthPlus, heightPlus, lineWidthPlus1;


  void drawPoints(void);
  void drawLines(void);


  void (Plot::* drawLineFunc)(int,int,int,int);
  void (Plot::* drawPointFunc)(int,int);
  

  void drawPoint0(int X, int Y);
  void drawLine0(int fromX, int fromY, int toX, int toY);

  // what plot number is this in the Graph.
  int plotCount;

  void init(void);

  void *pickerXDequeuer, *pickerYDequeuer;
};


class ZoomLevel
{
public:

  ~ZoomLevel(void);
  
  // For the first Zoom Level with the max and min limits given.
  ZoomLevel(Plot *plot_in, int width, int height,
            value_t xmax, value_t xmin,
            value_t ymax, value_t ymin);
  
  // Set the new zoom based on the zoom box and the current zoomLevel.
  ZoomLevel(Plot *plot, int width, int height,
            int xbox, int ybox, int wbox, int hbox);

  // Does not copy plot, next, and prev.  Copies scaleX, scaleY,
  // shiftX, shiftY, xMax, xMin, yMax, yMin.
  void copy(ZoomLevel *z);
  
  // Set the existing zoom based on the zoom box and the previous
  // zoomLevel.
  void reset(int width, int height,
             int xbox, int ybox, int wbox, int hbox);
  
  // Change the scales due to a resize of the window.
  void reset(int new_width, int new_height);
  
  // This reset() only works and makes sense for the first zoom level.
  // This is for when the Plots are all set to have the same scales.
  void reset(int width, int height,
             value_t xmax, value_t xmin,
             value_t ymax, value_t ymin);
    
  // scaledValue = scale * unscaledValue + shift
  // This is the inverse.
  inline void getValues(int x_in, int y_in, value_t *x_out, value_t *y_out)
    {
      *x_out = (((value_t) x_in) - shiftX)/scaleX;
      *y_out = (((value_t) y_in) - shiftY)/scaleY;
    }

  // This class is a data structure for a double-ly linked list.
  ZoomLevel *next, *prev;

    // xMax, xMin, yMax, yMin are used to set the scale and shift, and
    // are not maximum and minumum values of X and/or Y, but the
    // maximum and minumum values of X and/or Y in the Graph windows
    // view.  scaleX, scaleY, shiftX, shiftY are dependent on the
    // Graph window size, but xMax, xMin, yMax, yMin are not dependent
    // on the Graph window size.
  value_t scaleX, scaleY, shiftX, shiftY, xMax, xMin, yMax, yMin;
    
  Plot *plot;

private:
  

};

  
