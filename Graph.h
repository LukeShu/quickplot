/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */


class ColorGen;
class Field;
class MainWindow;

class Graph : public DrawingArea, public std::list<Plot *>
{
 public:

  Graph(MainWindow *mainWindow_in);
  void copy(Graph *graph);
  ~Graph(void);

  Plot *createPlot(Field *x, Field *y);
  void createDefaultPlots(Source *s);
  
  // Having a color generater for each graph will make the auto color
  // on all graphs behave similarly.
  ColorGen colorGen;

  void queueRedraw(void);

  void remove(Plot *plot);

  void checkScales(void);

  void drawAutoGrid(Glib::RefPtr<Gdk::Drawable> win,
                    Glib::RefPtr<Gdk::GC> gc);
  void savePNG(const char *filename);

  void zoomToTop(void);
  void zoomOut(void);
  void zoomIn(int x, int y, int w, int h);
  bool isZoomedOutToTop(void);

  void setShowLines(bool show);
  void setShowPoints(bool show);
  
  void setLineWidth(int width, bool oldPlotsToo=true);
  void setPointSize(int size,  bool oldPlotsToo=true);

  void setBackgroundColor(const Gdk::Color& color);
  void setGridColor(const Gdk::Color& color);

  void draw(Glib::RefPtr<Gdk::Drawable> win,
            Glib::RefPtr<Gdk::GC> gc);

  /************************************************************/
  /****************    Properties of a Graph     **************/
  /************************************************************/
  // Do the current plots have the same scale.
  bool isSameScale;
  
  // Do we force all plots to have the same scale.
  bool sameScale;
  
   // Do we make all plots the same scale if it works well.
  bool autoSameScale;

  // Draw the auto Grid.  This requires the Plots to have the same
  // scale (i.e. isSameScale is true).
  bool showAutoGrid;
  bool showGridNumbers;

  inline bool isShowingGrid() { return (showAutoGrid && isSameScale); }
  /************************************************************/

  static SigC::Signal1<void, Graph *> signal_changedSameScale(void);
  static SigC::Signal1<void, Graph *> signal_addedPlot(void);
  static SigC::Signal2<void, Graph *, Plot *> signal_removedPlot(void);
  SigC::Signal0<void> signal_backgroundColorChanged(void);

  
  Gdk::Color gridColor, backgroundColor;

  int gridXLineSpace, gridYLineSpace, gridLineWidth;

  int showLines, showPoints, lineWidth, pointSize;

  enum PICKER_TYPE
  {
    OFF_PLOT=0,
    INTERPOLATED=1,
    NONINTERPOLATED=2,
    NONE=3
  };
  
  PICKER_TYPE pickerType, highestPickerType;

  MainWindow *mainWindow;
  
protected:
  
  bool on_expose_event(GdkEventExpose*);
  bool on_button_press_event(GdkEventButton *event);
  bool on_button_release_event(GdkEventButton *event);
  bool on_motion_notify_event(GdkEventMotion *event);
  
private:

  void setPickerType(void);

  void setStatusXYValues(gdouble x, gdouble y);
  
  static SigC::Signal1<void, Graph *> m_signal_changedSameScale;
  
  static SigC::Signal1<void, Graph *> m_signal_addedPlot;
  static SigC::Signal2<void, Graph *, Plot *> m_signal_removedPlot;

  SigC::Signal0<void> m_signal_backgroundColorChanged;

  
  // used by createPlot(Field *x, Field *y) 
  void checkZoomLevelForNewPlot(Plot *plot);

  Glib::RefPtr<Gdk::Window> win;
  Glib::RefPtr<Gdk::GC> gc, inverted_gc;
  
  // used in Graph::drawAutoGrid()
  Glib::RefPtr<Pango::Layout> pangolayout;

  // for saving event state info
  guint buttonPressed;

  PICKER_TYPE lastPickerType;
  
  // Pointer x,y values when the button of interest was pressed
  int xPress, yPress;
  // The moving corner of the zoom box.
  int xmouse, ymouse, xpick, ypick;

  bool checkScalesQueued;
  bool isDeleting; // flag that says we are in ~Graph()

  int old_width, old_height;
};


class GraphTab : public HBox
{
public:

  GraphTab(int graphNumber, MainWindow *m, Graph *graph);

  void setText(const char *str);
  
  Label label;
  Button removeButton;
  Image closeImage;

  struct DeleteLater
  {
    GraphTab *graphTab;
  } deleteLater;
  
  MainWindow *mainWindow;
  Graph *graph;

private:

  void on_close(void);
  
};
