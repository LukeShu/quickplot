/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */


class GraphsNotebook : public Notebook
{
public:

  GraphsNotebook(MainWindow *mainWindow_in);

  // call this to copy the graphs from one notebook to another.
  void copy(GraphsNotebook *graphNotebook);
  // internal use only.  Needs to be public foe gtk callback stuff.
  void _copyLater(GraphsNotebook *graphNotebook);
  
  // this overrides Gtk::Notebook::set_show_tabs() because we need it
  // to "signal" the View Menu in MainMenuBar.cpp, to add a runtime
  // "show/hide tabs option.  Is there a signal_show_tabs() or like
  // method???  It's not listed.
  void set_show_tabs(bool show_tabs = true);
  
private:
  MainWindow *mainWindow;
};

class Source;
class PlotLister;
class PlotConfig;
class GraphConfig;
class Graph;
class StatusBar;
class Plot;

// Class MainWindow: is managed by App.  Use App::createMainWindow(),
// App::destroyMainWindow() and the globel currentMainWindow.  Do not
// call delete mainWindow. That is called in App::destroyMainWindow().
// Do not call 'new MainWindow', that is called in
// App::createMainWindow().


class MainWindow: public Window
{
 public:
  
  MainWindow(bool makeGraph=true);
  virtual ~MainWindow(void);

  void show(void);

  // Optional Graph Configuration Window.
  GraphConfig *graphConfig;
  Graph *currentGraph;
  PlotLister *plotLister;
  GraphsNotebook graphsNotebook;

  void savePNGFile(void);
  
  void showGraphConfig(void);
  void showPlotLister(void);
  void makePlotConfig(Plot *plot);
  void deletePlotConfig(PlotConfig *plotConfig);

  // the number of MainWindows that have been created when this one
  // was created, including this one.  So the first one is 1.
  int mainWindowNumber;

  
  void deleteLater(void);
  
protected:
  
  bool on_focus_in_event(GdkEventFocus* event);
  
  // looks at the files loaded and sets the title.
  void setTitle(Source *); // slot
  
  bool on_expose_event(GdkEventExpose *e);
  
private:
  VBox mainVBox, topVBox;

public:

  bool commonKeyPress(GdkEventKey* event);

  // create statusBar before menuBar
  StatusBar statusBar;
  MainMenuBar menuBar;
  ButtonBar buttonBar;

  void on_help(void); // slot
  void on_about(void); // slot
  void on_license(void); // slot
  
  
  void makeNewGraphTab(void); // slot
  void removeGraphTab(Graph *graph);
  
  void makeNewGraphTabWithGraph(Graph *graph);
  void makeNewGraphWithGraphConfig(void);
  void on_notebookFlip(GtkNotebookPage* page, guint page_num); //slot

private:

  int graphTabAddCount;
  
  void on_removedPlot(Graph *graph, Plot *plot);
  
  int numPlotConfigs;
  PlotConfig *plotConfigs[NUMPLOTCONFIGS];
  int plotConfigXY[2][NUMPLOTCONFIGS];


  // used to set the start up window geometry.
  int startX, startY, startXSign, startYSign;

  bool on_key_press_event(GdkEventKey* event);
};
