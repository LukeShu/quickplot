/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */

class PlotSelector;
class ValueSlider;

class GraphConfig : public Window
{
public:

  GraphConfig(MainWindow *m);

  void show();

  void setTitle(void);

private:

  
  void on_notebookFlip(GtkNotebookPage* , guint );

  void on_unmap(void);
  void on_hide(void);

  
  int x, y;
  
  MainWindow *mainWindow;

  VBox topVBox;
  Notebook notebook;
  Frame topLabelFrame;
  Label topLabel;
  HBox topHBox;
  VBox lVBox, rVBox;
  
  CheckButton
    showGridCB, showGridNumbersCB, showLinesCB, showPointsCB;
   
  OptionMenu sameScaleOM;
  Menu sameScaleM;
  Frame sameScaleStatusFrame;
  Label sameScaleStatusLabel;
  Button showPlotConfigB;

  VBox lowerVBox;
  HButtonBox colorHBox;
  Button bgColorB, gridColorB;

  HBox linePointHBox, grid1HBox, grid2HBox;
  ValueSlider lineWidthVS, pointSizeVS;
  ValueSlider gridLineWidthVS, gridFontSizeVS;
  LogValueSlider gridXLineSpaceVS, gridYLineSpaceVS;

public:
  PlotSelector plotSelector;

  void on_map(void);

private:

  void on_sameScaleOn(void);
  void on_sameScaleOff(void);
  void on_sameScaleAuto(void);
  void on_showGrid(void);
  void on_showGridNumbers(void);
  void on_showLines(void);
  void on_showPoints(void);
  void on_tabLabelChanged(Graph *graph);

  void on_sameScaleChange(Graph *graph);
  void on_showPlotConfig(void);
  
  // set widget state from graph
  void setValuesFromGraph(void);

  void on_bgColor(void);
  void on_gridColor(void);

  void on_gridXLineSpace(void);
  void on_gridYLineSpace(void);
  
  void on_gridLineWidth(void);
  void on_lineWidth(void);
  void on_pointSize(void);

  bool on_key_press_event(GdkEventKey* event);
};

