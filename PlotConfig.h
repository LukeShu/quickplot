/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */

#define NUMPLOTCONFIGS  2

#define PC_SMALL_INT  INT_MIN


class Plot;
class MainWindow;
class Picture;
class ValueSlider;

class PlotConfig: public Window
{
public:

  PlotConfig(MainWindow *mainWindow, Plot *plot, int x, int y);

  void setPlot(Plot *plot);

  void show(void);
  void on_unmap(void);
  void deleteLater(void);
  
  Plot *plot;
  int x, y;

private:
  
  MainWindow *mainWindow;


  VBox vBox;
  Picture picture;
  HBox hBox;

  // PLOT LINE
  Frame lineFrame;
  VBox lineVBox;
  CheckButton showLineCB;
  Button lineColorB;
  ValueSlider lineWidthVS;
  // RadioButton style1, style2 etc ...
  
  // PLOT POINTS
  Frame pointFrame;
  VBox pointVBox;
  CheckButton showPointsCB;
  Button pointColorB;
  ValueSlider pointSizeVS;
  // RadioButton style1, style2 etc ...

  void setTitle(void);

  SigC::Connection plotChanged_connection;

  void setValues(void);
  
  // slots
  void on_plotChanged(void);
  
  void on_showLine(void);
  void on_lineColor(void);
  void on_lineWidth(void);

  void on_showPoints(void);
  void on_pointColor(void);
  void on_pointSize(void);

  bool on_key_press_event(GdkEventKey* event);
};
