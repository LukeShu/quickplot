/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */
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
#include "GraphConfig.h"

#include "quickplot_icon.xpm"

#define SMALL_INT  INT_MIN


GraphConfig::GraphConfig(MainWindow *mainWindow_in):
  mainWindow(mainWindow_in),
  topLabel("Graph Configuration"),
  
  showGridCB("Show Grid"),
  showGridNumbersCB("Show Grid Numbers"),
  showLinesCB("Show Lines"),
  showPointsCB("Show Points"),
  showPlotConfigB("Configure Plots ..."),
  
  bgColorB("Background Color ..."),
  gridColorB("Grid Color ..."),

  lineWidthVS    (0, MAX_PLOT_LINE_WIDTH, MAXMAX_PLOT_LINE_WIDTH, "Plots Line Width"),
  pointSizeVS    (0, MAX_POINT_SIZE, MAXMAX_POINT_SIZE, "Point Size"),
  gridLineWidthVS(0, MAX_GRID_LINE_WIDTH, MAX_GRID_LINE_WIDTH, "Grid Line Width"),
  gridFontSizeVS(8, 40, 40, "Grid Font Size"),
  gridXLineSpaceVS(2, MAX_GRIDXLINESPACE, "Grid X Line Space"),
  gridYLineSpaceVS(2, MAX_GRIDYLINESPACE, "Grid Y Line Space"),
  plotSelector(this, mainWindow)
{
  set_position(WIN_POS_MOUSE);


  /********************* stacking the widgets ***********************/

  set_default_size(315, 600);
  set_border_width(5);
  //topVBox.set_border_width(5);
  topLabel.set_size_request(-1, 30);
  topVBox.set_spacing(20);
  topHBox.set_spacing(12);
  lVBox.set_spacing(5);
  rVBox.set_spacing(5);
  lowerVBox.set_spacing(5);
  linePointHBox.set_spacing(5);
  grid1HBox.set_spacing(5);
  grid2HBox.set_spacing(5);
  
  gridFontSizeVS.set_sensitive(false);
  gridFontSizeVS.setValue(10);
    
  //lFrame.set_shadow_type(SHADOW_ETCHED_IN);
  //rFrame.set_shadow_type(SHADOW_ETCHED_IN);
  //showGridOM.set_label("Show Grid");

     
  add(topVBox);
  topVBox.pack_start(topLabelFrame, PACK_SHRINK);
  topVBox.pack_start(topHBox, PACK_SHRINK);
  topVBox.pack_start(lowerVBox, PACK_SHRINK);
  topVBox.pack_start(plotSelector, PACK_EXPAND_WIDGET);
  
  topLabelFrame.add(topLabel);
  
  topHBox.pack_start(lVBox, PACK_EXPAND_WIDGET);
  topHBox.pack_start(rVBox, PACK_EXPAND_WIDGET);

  lVBox.pack_start(showGridCB, PACK_SHRINK);
  lVBox.pack_start(showGridNumbersCB, PACK_SHRINK);
  lVBox.pack_start(showLinesCB, PACK_SHRINK);
  lVBox.pack_start(showPointsCB, PACK_SHRINK);
  
  rVBox.pack_start(sameScaleStatusFrame, PACK_EXPAND_WIDGET);
  sameScaleStatusFrame.add(sameScaleStatusLabel);
  sameScaleStatusFrame.set_label("Plots Scaled the Same?");
  rVBox.pack_start(sameScaleOM, PACK_SHRINK);
  rVBox.pack_start(showPlotConfigB, PACK_SHRINK);

  lowerVBox.pack_start(colorHBox, PACK_SHRINK);
  colorHBox.pack_start(bgColorB, PACK_EXPAND_WIDGET);
  colorHBox.pack_start(gridColorB, PACK_EXPAND_WIDGET);
  
  lowerVBox.pack_start(linePointHBox, PACK_SHRINK);
  linePointHBox.add(lineWidthVS);
  linePointHBox.add(pointSizeVS);
  lowerVBox.pack_start(grid1HBox, PACK_SHRINK);
  grid1HBox.add(gridLineWidthVS);
  grid1HBox.add(gridFontSizeVS);
  lowerVBox.pack_start(grid2HBox, PACK_SHRINK);
  grid2HBox.add(gridXLineSpaceVS);
  grid2HBox.add(gridYLineSpaceVS);

  // same scale menu.

  sameScaleOM.set_menu(sameScaleM);
  Gtk::Menu::MenuList& menulist = sameScaleM.items();

  menulist.push_back( Gtk::Menu_Helpers::MenuElem("Same Scale",
    SigC::slot(*this, &GraphConfig::on_sameScaleOn) ) );
  
  menulist.push_back( Gtk::Menu_Helpers::MenuElem("Different Scales",
    SigC::slot(*this, &GraphConfig::on_sameScaleOff) ) );
  
  menulist.push_back( Gtk::Menu_Helpers::MenuElem("Automatic Scales",
    SigC::slot(*this, &GraphConfig::on_sameScaleAuto) ) );


  setValuesFromGraph();
  
  /******************************************************************/

  showGridCB.signal_clicked().
    connect( SigC::slot(*this, &GraphConfig::on_showGrid));
  showGridNumbersCB.signal_clicked().
    connect( SigC::slot(*this, &GraphConfig::on_showGridNumbers));
  showLinesCB.signal_clicked().
    connect( SigC::slot(*this, &GraphConfig::on_showLines));
  showPointsCB.signal_clicked().
    connect( SigC::slot(*this, &GraphConfig::on_showPoints));

  Graph::signal_changedSameScale().
    connect( SigC::slot(*this, &GraphConfig::on_sameScaleChange));

  bgColorB.signal_clicked().
    connect( SigC::slot(*this, &GraphConfig::on_bgColor));

  gridColorB.signal_clicked().
    connect( SigC::slot(*this, &GraphConfig::on_gridColor));
  
  gridXLineSpaceVS.signal_valueChanged().
    connect( SigC::slot(*this, &GraphConfig::on_gridXLineSpace));
  
  gridYLineSpaceVS.signal_valueChanged().
    connect( SigC::slot(*this, &GraphConfig::on_gridYLineSpace));
  
  gridLineWidthVS.signal_valueChanged().
    connect( SigC::slot(*this, &GraphConfig::on_gridLineWidth));

  lineWidthVS.signal_valueChanged().
    connect( SigC::slot(*this, &GraphConfig::on_lineWidth));
  
  pointSizeVS.signal_valueChanged().
    connect( SigC::slot(*this, &GraphConfig::on_pointSize));

  mainWindow->graphsNotebook.
    signal_switch_page().
    connect(SigC::slot(*this, &GraphConfig::on_notebookFlip));


  signal_show().connect(SigC::slot(mainWindow->menuBar,
                                   &MainMenuBar::checkGraphConfigState));
  signal_hide().connect(SigC::slot(mainWindow->menuBar,
                                   &MainMenuBar::checkGraphConfigState));
  
  signal_show().connect(SigC::slot(mainWindow->buttonBar,
                                   &ButtonBar::checkGraphConfigButton));
  signal_hide().connect(SigC::slot(mainWindow->buttonBar,
                                   &ButtonBar::checkGraphConfigButton));
  
  Glib::RefPtr<Gdk::Pixbuf> pix =
    Gdk::Pixbuf::create_from_xpm_data(quickplot_icon);
  set_icon(pix);
  
  show_all_children();

  x = SMALL_INT;
}

void GraphConfig::setValuesFromGraph(void)
{
  showGridCB.set_active(mainWindow->currentGraph->showAutoGrid);
  showGridNumbersCB.set_active(mainWindow->currentGraph->showGridNumbers);
  showLinesCB.set_active(mainWindow->currentGraph->showLines);
  showPointsCB.set_active(mainWindow->currentGraph->showPoints);
  
  gridXLineSpaceVS.setValue(mainWindow->currentGraph->gridXLineSpace);
  gridYLineSpaceVS.setValue(mainWindow->currentGraph->gridYLineSpace);
  gridLineWidthVS.setValue(mainWindow->currentGraph->gridLineWidth);

  lineWidthVS.setValue(mainWindow->currentGraph->lineWidth);
  pointSizeVS.setValue(mainWindow->currentGraph->pointSize);
  
  
  // sameScale
  if(mainWindow->currentGraph->sameScale)
  {
    sameScaleM.set_active(0);
    sameScaleOM.set_history(0);
  }
  else if(mainWindow->currentGraph->autoSameScale)
  {
    sameScaleM.set_active(2);
    sameScaleOM.set_history(2);
  }
  else
  {
    sameScaleM.set_active(1);
    sameScaleOM.set_history(1);
  }

  if(mainWindow->currentGraph->isSameScale)
    sameScaleStatusLabel.set_text("Yes");
  else
    sameScaleStatusLabel.set_text("No");
  
  setTabAsTitle();
}

void GraphConfig::on_notebookFlip(GtkNotebookPage* , guint )
{
  setValuesFromGraph();
}

void GraphConfig::setTabAsTitle(void)
{
  char s[16];
  
  if(mainWindow->mainWindowNumber > 1)
  {
    sprintf(s, "(%d) ", mainWindow->mainWindowNumber);
  }
  else
    s[0] = '\0';
  
  Glib::ustring str = s;
  
  GraphTab *l = dynamic_cast<GraphTab *>
    (mainWindow->graphsNotebook.get_tab_label(*(mainWindow->currentGraph)));
  if(l)
    str += l->label.get_text();
  
  str += ": Configure Graph";
  set_title(str);

  str = "Configure Graph: ";
  str += l->label.get_text();
  
  topLabel.set_text(str);
}

void GraphConfig::on_unmap()
{
  // Remember where the window was positioned if it is not showing
  // now.
  get_position(x , y);
  Window::on_unmap();
}

void GraphConfig::on_hide()
{
  // Don't know if this helps or not.
  // Remember where the window was positioned if it is not showing
  // now.
  //if(is_visible())
  //get_position(x , y);
  
  Window::on_hide();
}


void GraphConfig::show()
{
  // Remember where the window was positioned.
  if(!is_visible() && x != SMALL_INT)
    move(x, y);
  
  Window::show();
}

void GraphConfig::on_sameScaleOn(void)
{
  //opSpew << "GraphConfig::on_sameScaleOn()" << std::endl;
  
  if(!(mainWindow->currentGraph->sameScale))
  {
    mainWindow->currentGraph->sameScale = true;
    if(!(mainWindow->currentGraph->isSameScale))
    {
      mainWindow->currentGraph->checkScales();
    }
  }
}

void GraphConfig::on_sameScaleOff(void)
{
  //opSpew << "GraphConfig::on_sameScaleOff()" << std::endl;
  
  if(mainWindow->currentGraph->sameScale ||
     mainWindow->currentGraph->autoSameScale)
  {
    mainWindow->currentGraph->sameScale = false;
    mainWindow->currentGraph->autoSameScale = false;
    if(mainWindow->currentGraph->isSameScale &&
       mainWindow->currentGraph->size() > 1)
    {
      mainWindow->currentGraph->checkScales();
    }
  }
}

void GraphConfig::on_sameScaleAuto(void)
{
  //opSpew << "GraphConfig::on_sameScaleAuto()" << std::endl;
  
  if(!(mainWindow->currentGraph->autoSameScale) ||
     mainWindow->currentGraph->sameScale)
  {
    mainWindow->currentGraph->sameScale = false;
    mainWindow->currentGraph->autoSameScale = true;
    if(mainWindow->currentGraph->size() > 1)
    {
      mainWindow->currentGraph->checkScales();
    }
  }
}

void GraphConfig::on_sameScaleChange(Graph *graph)
{
  if(graph == mainWindow->currentGraph)
  {
    if(graph->isSameScale)
      sameScaleStatusLabel.set_text("Yes");
    else
      sameScaleStatusLabel.set_text("No");
  }
}

void GraphConfig::on_showGrid(void)
{
  int isShowingGrid = mainWindow->currentGraph->isShowingGrid();
  if(showGridCB.get_active() != mainWindow->currentGraph->showAutoGrid)
  {
    mainWindow->currentGraph->showAutoGrid = showGridCB.get_active();
    
    if((isShowingGrid && !showGridCB.get_active())
       ||
       (!isShowingGrid && showGridCB.get_active())
       )
    {
      mainWindow->currentGraph->queueRedraw();
    }
  }
}

void GraphConfig::on_showGridNumbers(void)
{
  mainWindow->currentGraph->showGridNumbers = showGridNumbersCB.get_active();
  mainWindow->currentGraph->queueRedraw();
}

void GraphConfig::on_showLines(void)
{
  mainWindow->currentGraph->setShowLines(showLinesCB.get_active());
  plotSelector.drawArea.queueRedraw();
}

void GraphConfig::on_showPoints(void)
{
  mainWindow->currentGraph->setShowPoints(showPointsCB.get_active());
  plotSelector.drawArea.queueRedraw();
}

void GraphConfig::on_gridXLineSpace(void)
{
  mainWindow->currentGraph->gridXLineSpace = gridXLineSpaceVS.getValue();
  mainWindow->currentGraph->queueRedraw();
}

void GraphConfig::on_gridYLineSpace(void)
{
  mainWindow->currentGraph->gridYLineSpace = gridYLineSpaceVS.getValue();  
  mainWindow->currentGraph->queueRedraw();
}

void GraphConfig::on_gridLineWidth(void)
{
  mainWindow->currentGraph->gridLineWidth = gridLineWidthVS.getValue();
  mainWindow->currentGraph->queueRedraw();
}

void GraphConfig::on_lineWidth(void)
{
  mainWindow->currentGraph->setLineWidth(lineWidthVS.getValue());
  mainWindow->currentGraph->queueRedraw();
  plotSelector.drawArea.queueRedraw();
}

void GraphConfig::on_pointSize(void)
{
  mainWindow->currentGraph->setPointSize(pointSizeVS.getValue());
  mainWindow->currentGraph->queueRedraw();
  plotSelector.drawArea.queueRedraw();
}

void GraphConfig::on_showPlotConfig(void)
{
}  

void GraphConfig::on_bgColor(void)
{
  Gtk::ColorSelectionDialog dialog;

  //Set the current color:
  Gtk::ColorSelection* pColorSel = dialog.get_colorsel();

  { // set dialog title
    char s[16];
    if(mainWindow->mainWindowNumber > 1)
    {
      sprintf(s, "(%d) ", mainWindow->mainWindowNumber);
    }
    else
      s[0] = '\0';
    
    Glib::ustring str = s;
    str += "Graph Background Color: ";
    GraphTab *l = dynamic_cast<GraphTab *>
      (mainWindow->graphsNotebook.get_tab_label(*(mainWindow->currentGraph)));
    if(l)
      str += l->label.get_text();
    dialog.set_title(str);
  }
  
  pColorSel->set_current_color(mainWindow->currentGraph->backgroundColor);
  
  int result = dialog.run();
  
  //Handle the response:
  switch(result)
  {
    case(Gtk::RESPONSE_OK):
    {
      const Gdk::Color color = pColorSel->get_current_color();
      mainWindow->currentGraph->setBackgroundColor(color);
      mainWindow->currentGraph->queueRedraw();
      plotSelector.drawArea.get_window()->
        set_background(mainWindow->currentGraph->backgroundColor);
      plotSelector.drawArea.queueRedraw();
      break;
    }
    case(Gtk::RESPONSE_CANCEL):
    {
      //opSpew << "Cancel clicked." << std::endl;
      break;
    }
    default:
    {
      //opSpew << "Unexpected button clicked." << std::endl;
      break;
    }
  }

}

void GraphConfig::on_gridColor(void)
{
  Gtk::ColorSelectionDialog dialog;

  //Set the current color:
  Gtk::ColorSelection* pColorSel = dialog.get_colorsel();
  pColorSel->set_current_color(mainWindow->currentGraph->gridColor);
  
  { // set dialog title
    char s[16];
    if(mainWindow->mainWindowNumber > 1)
    {
      sprintf(s, "(%d) ", mainWindow->mainWindowNumber);
    }
    else
      s[0] = '\0';
    
    Glib::ustring str = s;
    str += "Graph Grid Color: ";
    GraphTab *l = dynamic_cast<GraphTab *>
      (mainWindow->graphsNotebook.get_tab_label(*(mainWindow->currentGraph)));
    if(l)
      str += l->label.get_text();
    dialog.set_title(str);
  }
  
  int result = dialog.run();
  
  //Handle the response:
  switch(result)
  {
    case(Gtk::RESPONSE_OK):
    {
      const Gdk::Color color = pColorSel->get_current_color();
      mainWindow->currentGraph->setGridColor(color);
      mainWindow->currentGraph->queueRedraw();
      plotSelector.drawArea.queueRedraw();
      break;
    }
    case(Gtk::RESPONSE_CANCEL):
    {
      //opSpew << "Cancel clicked." << std::endl;
      break;
    }
    default:
    {
      //opSpew << "Unexpected button clicked." << std::endl;
      break;
    }
  }

}
