/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */
#include "config.h"

#include <list>
#include <iomanip>

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


#include "errorStr.h"
#include "Globel.h"
#include "Field.h"
#include "Source.h"
#include "FileList.h"
#include "File.h"
#include "Plot.h"
#include "ColorGen.h"
#include "Graph.h"
#include "PlotSelector.h"

#define SMALL_INT  INT_MIN


PlotSelector::PlotSelector(Window *graphConfig,
                           MainWindow *mainWindow_in) :
  topLabel("Select Fields to Plot or Unplot"),
  xLabel("      X Field      "),
  blankLabel("   "),
  yLabel("      Y Field      "),
  //scrolledWindow(graphConfig, &scrolledHBox),
  xNoneRadioButton("none"),
  yNoneRadioButton("none"),
  drawArea(this)
{
  mainWindow = mainWindow_in;

  scrolledWindow.set_policy(POLICY_AUTOMATIC, POLICY_AUTOMATIC);
  scrolledWindow.set_size_request(-1, 100);
  drawArea.set_size_request(30, 10);
  topLabel.set_size_request(-1, 30);
  set_shadow_type(SHADOW_ETCHED_IN);
  
  //labelBox.set_size_request(-1, 40);

  add(topVBox);
  topVBox.pack_start(topLabelFrame, PACK_SHRINK);
  topLabelFrame.add(topLabel);
  
  topVBox.pack_start(labelBox, PACK_SHRINK);
  topVBox.add(scrolledWindow);
  labelBox.pack_start(xLabel, PACK_SHRINK);
  labelBox.pack_start(blankLabel, PACK_EXPAND_PADDING);
  labelBox.pack_start(yLabel, PACK_SHRINK);
  scrolledWindow.add(scrolledHBox);
  drawAreaFrame.add(drawArea);
  
  scrolledHBox.pack_start(xBox, PACK_SHRINK);
  scrolledHBox.pack_start(drawAreaFrame, PACK_EXPAND_WIDGET);
  scrolledHBox.pack_start(yBox, PACK_SHRINK);
  
  xBox.pack_start(xFrame, PACK_SHRINK);
  xFrame.add(xNoneRadioButton);
  
  yBox.pack_start(yFrame, PACK_SHRINK);
  yFrame.add(yNoneRadioButton);

  Source::signal_addedSource().
    connect(SigC::slot(*this, &PlotSelector::on_addSource));
  Source::signal_removedSource().
    connect(SigC::slot(*this, &PlotSelector::on_removeSource));

  mainWindow->graphsNotebook.
    signal_switch_page().
    connect(SigC::slot(*this, &PlotSelector::on_notebookFlip));
  
  std::list<Source *>::const_iterator source = sources.begin();
  for(;source != sources.end(); source++)
  {    
    xBox.pack_start(*newSourceButtons(X, *source), PACK_SHRINK);
    yBox.pack_start(*newSourceButtons(Y, *source), PACK_SHRINK);
  }

  fixNoneButtons = false;
}

PlotSelector::~PlotSelector(void)
{
  // delete all the dynmaically allocated widgets in xBox and yBox.
  {
    Glib::ListHandle<Widget *> list = xBox.get_children();
    Glib::ListHandle<Widget *>::iterator it = list.begin();
    for(it++;it != list.end(); it++)
    {
      Container *c = dynamic_cast<Container *>(*it);
      if(c)
        deleteWidgetChildren(c);
      else
        delete *it;
    }
  }
  {
    Glib::ListHandle<Widget *> list = yBox.get_children();
    Glib::ListHandle<Widget *>::iterator it = list.begin();
    for(it++;it != list.end(); it++)
    {
      Container *c = dynamic_cast<Container *>(*it);
      if(c)
        deleteWidgetChildren(c);
      else
        delete *it;
    }
  }
}

void PlotSelector::on_notebookFlip(GtkNotebookPage* , guint )
{
  fixNoneButtons = true;
  // fix the window to match the currentGraph.
  drawArea.queueRedraw();
  queueRedraw();
  
  if(xNoneRadioButton.activate() &&
     yNoneRadioButton.activate() &&
     xNoneRadioButton.get_active() &&
     yNoneRadioButton.get_active())
  {
    fixNoneButtons = false;
  }
}

// recursively delete all children.
void PlotSelector::deleteWidgetChildren(Container *container)
{ 
  Glib::ListHandle<Widget *> list = container->get_children();
  Glib::ListHandle<Widget *>::iterator it = list.begin();
  for(;it != list.end(); it++)
  {
    Label *l = dynamic_cast<Label *>(*it);
    if(l)
      labels.remove(l);

    FieldButton *fb = dynamic_cast<FieldButton *>(*it);
    if(fb)
    {
      if(fb->xory == X)
        xFieldButtons.remove(fb);
      else // fb->xory == Y
        yFieldButtons.remove(fb);
    }
    
    Container *c = dynamic_cast<Container *>(*it);
    if(c)
    {
      deleteWidgetChildren(c);
      container->remove(*c);
    }
    delete *it;
  }
}


bool PlotSelector::on_expose_event(GdkEventExpose *e)
{
  if(fixNoneButtons)
  {
    if(xNoneRadioButton.get_active() &&
       yNoneRadioButton.get_active())
    {
      fixNoneButtons = false;
    }
    else
      if(xNoneRadioButton.activate() &&
         yNoneRadioButton.activate() &&
         xNoneRadioButton.get_active() &&
         yNoneRadioButton.get_active())
      {
        fixNoneButtons = false;
      }
  }
  
  //opSpew << "---------------------------------------------------------------------" << std::endl;
  //opSpew << "scrolledHBox.get_height()=" << scrolledHBox.get_height()<< std::endl;
  //opSpew << "scrolledWindow.get_height()=" << scrolledWindow.get_height() << std::endl;

  return Frame::on_expose_event(e);
}

void PlotSelector::queueRedraw(void)
{
  GdkRectangle rec =
    {
      0, 0, get_width(), get_height()
    };
  
  gdk_window_invalidate_rect(get_window()->gobj(), &rec, true);
  //gdk_window_process_updates(get_window()->gobj(), true);
}


// Call this if there is a change in the Sources/Files loaded.
void PlotSelector::on_addSource(Source *s)
{
  xBox.pack_start(*newSourceButtons(X, s), PACK_SHRINK);
  yBox.pack_start(*newSourceButtons(Y, s), PACK_SHRINK);

  
  drawArea.queueRedraw();
  
  show_all_children();


  fixNoneButtons = true;

  //queueRedraw();

}

void PlotSelector::on_removeSource(Source *s)
{
  {
    // look for the X SourceFrame with Source *s.
    Glib::ListHandle<Widget *> list = xBox.get_children();
    Glib::ListHandle<Widget *>::iterator it = list.begin();
    for(;it != list.end(); it++)
    {
      SourceFrame *sf = dynamic_cast<SourceFrame *>(*it);
      if(sf && sf->source == s)
      {
        // remove and delete it.
        xBox.remove(*sf);
        deleteWidgetChildren(sf);
        delete sf;
        break;
      }
    }
  }
  {
    // look for the Y SourceFrame with Source *s.
    Glib::ListHandle<Widget *> list = yBox.get_children();
    Glib::ListHandle<Widget *>::iterator it = list.begin();
    for(;it != list.end(); it++)
    {
      SourceFrame *sf = dynamic_cast<SourceFrame *>(*it);
      if(sf && sf->source == s)
      {
        // remove and delete it.
        yBox.remove(*sf);
        deleteWidgetChildren(sf);
        delete sf;
        break;
      }
    }
  }
  
  fixNoneButtons = true;

  drawArea.queueRedraw();
}

FieldButton *PlotSelector::newFieldButton(XY xy, Source *s,
                                         Field *f, const char *label)
{
  FieldButton *pb = new FieldButton(this, xy, s, f, label);
  if(xy == X)
  {
    xFieldButtons.push_back(pb);
    RadioButton::Group g = xNoneRadioButton.get_group();
    pb->set_group(g);
  }
  else // xy == Y
  {
    yFieldButtons.push_back(pb);
    RadioButton::Group g = yNoneRadioButton.get_group();
    pb->set_group(g);
  }

  return pb;
}

Frame *PlotSelector::newSourceButtons(PlotSelector::XY xy, Source *source)
{
  Frame *f = new SourceFrame(source); 
  VBox *vBox = new VBox;
  f->add(*vBox);
  Label *l = new Label(source->getBaseFileName());
  labels.push_back(l);
  vBox->pack_start(*l, PACK_SHRINK);
  HSeparator *hs = new HSeparator;
  vBox->pack_start(*hs, PACK_SHRINK);

  std::list<Field *>::const_iterator field = source->begin();
  for(;field != source->end(); field++)
  {
    //opSpew << (*field)->getLabel() << " " << (*field)->getName() << endl;
    Glib::ustring str(((*field)->getLabel())?(*field)->getLabel():"");
    str += " (";
    str += ((*field)->getName())?(*field)->getName():"";
    str += ")";
    
    FieldButton *b = newFieldButton(xy, source, (*field), str.c_str());
    vBox->pack_start(*b, PACK_SHRINK);

    b->signal_toggled().
      connect(SigC::slot(*b, &FieldButton::clicked));
  }
  return f;
}

void PlotSelector::radioButtonClicked(FieldButton *b)
{
  if(!b->get_active()) return;
  
  //opSpew << "PlotSelector::on_radioClicked(" << b << ")" << std::endl;
  Field *xField =NULL, *yField=NULL;
  RadioButton *xButton = NULL, *yButton=NULL;

  if(b->xory == X)
  {
    xField = b->field;
    xButton = b;
  }
  else // b->xory == Y
  {
    yField = b->field;
    yButton = b;
  }

  if(xField) // look for the Y field
  {
    std::list<FieldButton *>::const_iterator it = yFieldButtons.begin();
    for(;it != yFieldButtons.end(); it++)
    {
      if((*it)->get_active())
      {
        yField = (*it)->field;
        yButton = (*it);
        break;
      }
    }
  }
  else // look for the X field
  {
    std::list<FieldButton *>::const_iterator it = xFieldButtons.begin();
    for(;it != xFieldButtons.end(); it++)
    {
      if((*it)->get_active())
      {
        xField = (*it)->field;
        xButton = (*it);
        break;
      }
    }
  }

  // If there two fields are not selected return.
  if(!xField || !yField) return;
  
  // Delete the Plot if it exists
  std::list<Plot *>::const_iterator plot_it;
  for(plot_it=mainWindow->currentGraph->begin();
      plot_it != mainWindow->currentGraph->end(); plot_it++)
  {
    if((*plot_it)->x() == xField && (*plot_it)->y() == yField)
    {
      delete (*plot_it);
      drawArea.queueRedraw();
      return;
    }
  }

  /****************************************************/
  /*                 Now add a plot                   */
  /****************************************************/
  Plot *plot = mainWindow->currentGraph->createPlot(xField, yField);
  
  drawArea.drawPlotLine(plot, xButton, yButton);


  if(opVerbose)
    opSpew << "plotting: "
           << yField->getLabel() << " (" << yField->getName() << ")"
           << " VS "
           << xField->getLabel() << " (" << xField->getName() << ")"
           << std::endl;  
}


void FieldButton::clicked(void)
{
  plotSelector->radioButtonClicked(this);
}

FieldButton::FieldButton(PlotSelector *ps, PlotSelector::XY xy, Source *s,
                         Field *f, const char *label):
  RadioButton(label),
  xory(xy),
  plotSelector(ps),
  source(s), field(f)
{
}


SourceFrame::SourceFrame(Source *s):
  source(s)
{
}

ConnectFieldsDrawingArea::
ConnectFieldsDrawingArea(PlotSelector *ps):
  plotSelector(ps)
{
  win.clear();
  gc.clear();
}

void ConnectFieldsDrawingArea::queueRedraw(void)
{
  GdkRectangle rec =
    {
      0, 0, get_width(), get_height()
    };
  
  gdk_window_invalidate_rect(get_window()->gobj(), &rec, true);
}

void ConnectFieldsDrawingArea::
drawPlotLine(Plot *plot,
             RadioButton *xRadioButton,
             RadioButton *yRadioButton)
{
  GtkAllocation a = get_allocation();
  int yoffset = a.y;
  a = xRadioButton->get_allocation();
  int left_y = a.y + a.height/2 - yoffset;
  a = yRadioButton->get_allocation();
  int right_y = a.y + a.height/2 - yoffset;

  
  // draw a line
  {
    int lineWidth;
    
    if(plot->getShowLines() || !plot->getShowPoints()) // use line color
    {
      lineWidth = (plot->getLineWidth() < 11) ? plot->getLineWidth() : 11;
      gc->set_foreground(plot->getLineColor());
    }
    else // use the points color for the lines
    {
      lineWidth = plot->getPointSize();
      if(lineWidth > 11) lineWidth = 11;
      gc->set_foreground(plot->getPointColor());
    }
    
    gc->set_line_attributes (lineWidth, Gdk::LINE_SOLID,
                             Gdk::CAP_ROUND, Gdk::JOIN_ROUND);
    
    win->draw_line(gc, 0, left_y, get_width(), right_y);
  }
  
  if(plot->getShowPoints() && plot->getShowLines())
  {
    // put a 2 points in the line

    int pointSize = (plot->getPointSize() < 11) ? plot->getPointSize() : 11;
    
    gc->set_foreground(plot->getPointColor());

    if(pointSize <= 1)
    {
      win->draw_point(gc, get_width()/3, left_y + (right_y-left_y)/3 );
      win->draw_point(gc, 2*get_width()/3, left_y + 2*(right_y-left_y)/3 );
    }
    else
    {
      win->draw_rectangle(gc, true,
                          get_width()/3 - pointSize/2,
                          left_y+(right_y-left_y)/3 - pointSize/2,
                          pointSize, pointSize);
      win->draw_rectangle(gc, true,
                          2*get_width()/3 - pointSize/2,
                          left_y + 2*(right_y-left_y)/3 - pointSize/2,
                          pointSize, pointSize);
    }
  }
}


bool ConnectFieldsDrawingArea::on_expose_event(GdkEventExpose*)
{
  if(win.is_null())
  {
    win = get_window();
    gc = Gdk::GC::create(win);
  }

  win->set_background(plotSelector->mainWindow->currentGraph->backgroundColor);
  win->clear();
  
  int yoffset = get_allocation().y;
  
  // Draw the little blob on the edges of the radio buttons.
  gc->set_foreground(plotSelector->mainWindow->currentGraph->gridColor);
  
  std::list<FieldButton *>::const_iterator it =
    plotSelector->xFieldButtons.begin();
  for(;it != plotSelector->xFieldButtons.end(); it++)
  {
    int y = (*it)->get_allocation().y - yoffset;
    int h = (*it)->get_height();
    win->draw_arc(gc, true, -h/4, y+h/4, h/2, h/2, -90*64, 64*180);
  }
  
  it = plotSelector->yFieldButtons.begin();
  for(;it != plotSelector->yFieldButtons.end(); it++)
  {
    int y = (*it)->get_allocation().y - yoffset;
    int h = (*it)->get_height();
    win->draw_arc(gc, true, get_width()-h/4, y+h/4, h/2, h/2, 180*64, 64*360);
  }
  
  std::list<Label *>::const_iterator lit =
    plotSelector->labels.begin();
  for(;lit != plotSelector->labels.end(); lit++)
  {
    int y = (*lit)->get_allocation().y - yoffset;
    int h = (*lit)->get_height();
    win->draw_rectangle(gc, true, 0, y, get_width(), h);
  }
  
  // Add lines that connect the RadioButtons that corrispond to each
  // plot.
  std::list<Plot *>::const_iterator plot, end;
  end = plotSelector->mainWindow->currentGraph->end();
  for(plot=plotSelector->mainWindow->currentGraph->begin();
      plot != end; plot++)
  {
    // Find the RadioButtons for a given plot.
    std::list<FieldButton *>::const_iterator xb;
    for(xb=plotSelector->xFieldButtons.begin();
        xb != plotSelector->xFieldButtons.end(); xb++)
      if((*xb)->field == (*plot)->x())
        break;

    std::list<FieldButton *>::const_iterator yb;
    for(yb=plotSelector->yFieldButtons.begin();
        yb != plotSelector->yFieldButtons.end(); yb++)
      if((*yb)->field == (*plot)->y())
        break;

    if(xb != plotSelector->xFieldButtons.end() &&
       yb != plotSelector->yFieldButtons.end())
      // Draw a line to join the RadioButtons
      drawPlotLine(*plot, *xb, *yb);
  }
  
  return true;
}

#if 0
// This needs the Window and other Containing Widgets.  This is a Each
// time the queueRedraw() is called this resizes Window making the
// child widget show completely if the Window height is less than
// 1000.
// This does not work all the time, so it's not used for now.
SizingScrolledWindow::SizingScrolledWindow(Window *window_in,
                                           Widget *child_in):
  window(window_in),
  child(child_in)
{
  set_size_request(-1, 10);
  needsResizing = true;
  resizeCount = 0;

  set_policy(POLICY_NEVER, POLICY_AUTOMATIC);
  
  // The child widget height can be larger than the parent, of parents
  // being ScrolledWindows.
  
  childHeight = SMALL_INT; // min height of child Widget
  diffHeight = SMALL_INT; // get_height() - child Height when window
                          // height is large.
}

// returns 1 for needs more work, returns 0 for okay.
int SizingScrolledWindow::checkSize(void)
{
  // the resize() calls in here don't take effect for many loops of
  // this.  Usually about 11.  So 200 is enough.  This is nuts.
  resizeCount++;
  if(resizeCount > 200) // keep from infinite loop.
  {
    needsResizing = false;
    resizeCount = 0;
    return 0;
  }
  
  if(childHeight != SMALL_INT &&
     get_height() <= child->get_height() &&
     window->get_height() >= 1000)
  {
    needsResizing = false;
    return 0;
  }
  
  if(childHeight == SMALL_INT)
  {    
    if(get_height() < child->get_height())
    {
      //opSpew << __LINE__ <<  "  file=" << __FILE__ << std::endl;
      childHeight = child->get_height();
    }
    else 
    {
      //opSpew << __LINE__ <<  "  file=" << __FILE__ << std::endl;
      window->resize(window->get_width(), 12);
      return 1; // find it next time this is called.
    }   
  }
      
  // here we have childHeight set.
  //opSpew << __LINE__ <<  "  file=" << __FILE__ << std::endl;
    
  if(diffHeight == SMALL_INT && get_height() < childHeight)
  {
    // Set this so we can get the diffHeight
    window->resize(window->get_width(), window->get_height() + childHeight + 40);
    return 1;// find it next time this is called.
  }
  
  if(diffHeight == SMALL_INT)
  {
    diffHeight = get_height() - child->get_height();
  }

  // here we have childHeight and diffHeight
  
  if(get_height() != childHeight + diffHeight)
  {
    int h = window->get_height() +
      (childHeight + diffHeight) - get_height();

    if(h > 1000 && oldWindowHeight > h)
    {
      if(h > oldWindowHeight)
        h = oldWindowHeight;
    }
    else if(h > 1000 && oldWindowHeight < 1000)
      h = 1000;
    //opSpew << __LINE__ <<  "  file=" << __FILE__ << std::endl;

    window->resize(window->get_width(), h);
  }

  needsResizing = false;
  return 0;
}

bool SizingScrolledWindow::on_expose_event(GdkEventExpose *e)
{
  if(needsResizing)
    checkSize();
  
  bool retVal =  ScrolledWindow::on_expose_event(e);
  
  return  retVal;
}

void SizingScrolledWindow::queueRedraw(bool needsResizing_in)
{
  if(needsResizing_in)
  {
    needsResizing = true;
    oldWindowHeight = window->get_height();
    childHeight = SMALL_INT;
    resizeCount = 0;
  }
  
  
  GdkRectangle rec =
    {
      0, 0, get_width(), get_height()
    };
  
  gdk_window_invalidate_rect(get_window()->gobj(), &rec, true);
  //gdk_window_process_updates(get_window()->gobj(), true);
}
#endif
