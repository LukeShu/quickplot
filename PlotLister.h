/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */

class MainWindow;
class Plot;
class Graph;

class Picture:  public DrawingArea
{
public:
  
  Picture(Plot *plot=NULL, bool limit_size=false);
  ~Picture(void);
  
  bool on_expose_event(GdkEventExpose *);

  void queueRedraw(void);
  void setPlot(Plot *p);
  
private:
  
  Plot *plot;
  bool limitSize;
  
  Glib::RefPtr<Gdk::Window> win;
  Glib::RefPtr<Gdk::GC> gc;

  SigC::Connection plotChangeConnection, bgConnection;
};



class Row
{
public:
  
  Row(MainWindow *mainWindow, Table *table_in, int row_in, Plot *plot);

  void on_valueDisplay(value_t x, value_t y);
  void printStdout(void);
  
private:

  Button plotConfigB;
  Entry labelE, valueE;
  Label xMinE, xMaxE, yMinE, yMaxE;


  Picture picture;
  
  int row;
  Table *table;
  MainWindow *mainWindow;
  Plot *plot;
  
  void add(Widget &w, int col);
  void makePlotConfig(void);
  //void setLabelSize(Entry &l);
  //void afterRealize(void);

  //SigC::Connection afterRealizeConnection;
};



class PlotLister : public Window
{

public:
  PlotLister(MainWindow *mainWindow_in);
  ~PlotLister(void);

  void show(void);
  void on_unmap(void);
  void setTitle(void);

private:
  MainWindow *mainWindow;


  void on_notebookFlip(GtkNotebookPage* , guint );


  void setValuesFromGraph(void);

  void addHeaderLabel(Widget &w, int col);

  void on_addedPlot(Graph *graph);

  void on_removedPlot(Graph *graph, Plot *plot);

  void on_interpolated(void);
  void on_noninterpolated(void);
  void on_offPlot(void);

  int x, y;

private:

  void on_printToStdout(void);

  Row **row;
  unsigned int  numRows, rowArraySize;

  VBox vBox;
  HBox hBox;
  OptionMenu pickerTypeOM;
  Menu pickerTypeM;
  MenuItem  offPlotMI, interpolatedMI, noninterpolatedMI;

  Button printToStdoutB;
  ScrolledWindow scrolledWindow;
  Table table;

  Label plotLabelL, plotPictureL, valuesL, xMinL, xMaxL, yMinL, yMaxL;

  bool on_key_press_event(GdkEventKey* event);
};
