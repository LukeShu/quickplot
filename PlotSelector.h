/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */

class FieldButton;
class Source;
class PlotSelector;
class MainWindow;

#if 0
class SizingScrolledWindow: public ScrolledWindow
{
public:

  SizingScrolledWindow(Window *window,
                       Widget *child);
  
  void queueRedraw(bool needsResizing=true);

  
private:
  
  bool on_expose_event(GdkEventExpose *e);

  // returns 1 for needs more work, returns 0 for Okay.
  int checkSize(void);

  Window *window;
  Widget *child;

  int diffHeight, childHeight, oldWindowHeight, resizeCount;


  bool needsResizing;
};
#endif

class ConnectFieldsDrawingArea : public DrawingArea
{
public:
  ConnectFieldsDrawingArea(PlotSelector *ps);
  
  bool on_expose_event(GdkEventExpose*);
  
  void drawPlotLine(Plot *plot,
                    RadioButton *xRadioButton,
                    RadioButton *yRadioButton);

  void queueRedraw(void);

private:

  Glib::RefPtr<Gdk::Window> win;
  Glib::RefPtr<Gdk::GC> gc;
  Gdk::Color blobColor;


  PlotSelector *plotSelector;
};


extern "C"
{
  struct FixNoneButtons
  {
    PlotSelector *plotSelector;
  };
}


class PlotSelector : public VBox
{
public:
  
  PlotSelector(Window *graphConfig, MainWindow *mainWindow_in);
  virtual ~PlotSelector(void);
  
  std::list<FieldButton *> xFieldButtons;
  std::list<FieldButton *> yFieldButtons;
  std::list<Label *> labels;
 
  enum XY { X, Y};
  
  void radioButtonClicked(FieldButton *b);

  void queueRedraw(void);

protected:

  // We need to connect the signals after the widget is first drawn.
  virtual void on_map(void);
  
private:

  bool on_expose_event(GdkEventExpose *e);
  void on_addSource(Source *s);
  void on_removeSource(Source *s);

  RadioButton *newFieldButton(XY xy, Source *s, Field *f);

  Frame topLabelFrame;
  Label topLabel;
  
  HBox labelBox;
  Label xLabel, blankLabel, yLabel;
  HBox scrolledHBox;
  //SizingScrolledWindow scrolledWindow;
  
  ScrolledWindow scrolledWindow;
  
  VBox xBox, yBox;
  Frame xFrame, yFrame;
  RadioButton xNoneRadioButton, yNoneRadioButton;
  Frame drawAreaFrame;

public:

  ConnectFieldsDrawingArea drawArea;

  bool wasMapped;

private:
 
  int x,y;

    // make a new FieldButton and add it to the list.
  FieldButton *newFieldButton(XY XorY, Source *s,
                             Field *f, const char *label);

  Frame *newSourceButtons(XY xy, Source *s);

  void deleteWidgetChildren(Container *c);

  void on_notebookFlip(GtkNotebookPage* , guint );

  bool fixNoneButtons;

  MainWindow *mainWindow;

  friend class ConnectFieldsDrawingArea;
};


class FieldButton : public RadioButton
{
public:

  FieldButton(PlotSelector *ps,
              PlotSelector::XY xy, Source *s,
              Field *f, const char *label);
  
  void clicked(void);

  PlotSelector::XY xory;

  PlotSelector *plotSelector;
  Source *source;
  Field *field;

};

class SourceFrame : public Frame
{
public:

  SourceFrame(Source *s);

  Source *source;
};

  
