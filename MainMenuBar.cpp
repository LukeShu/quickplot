/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */

#include <list>
#include <values.h>

#include <gtkmm.h>


using namespace Gtk;
#include "errorStr.h"
#include "value_t.h"
#include "Field.h"
#include "Plot.h"
#include "ColorGen.h"
#include "Graph.h"
#include "PlotSelector.h"
#include "ValueSlider.h"
#include "PlotLister.h"
#include "PlotConfig.h"
#include "GraphConfig.h"
#include "Globel.h"

#include "MainMenuBar.h"
#include "ButtonBar.h"
#include "StatusBar.h"
#include "MainWindow.h"
#include "App.h"

#include "Source.h"
#include "FileList.h"
#include "File.h"
#include "about_browser16x16.xpm"
#include "newFrame16x16.xpm"
#include "closeFrame16x16.xpm"
#include "copyFrame16x16.xpm"
#include "saveImage16x16.xpm"

MainMenuBar::MainMenuBar(MainWindow *mainWindow_in) :
  openImage(Stock::OPEN, ICON_SIZE_MENU),
  newImage(Stock::NEW, ICON_SIZE_MENU),
  quitImage(Stock::QUIT, ICON_SIZE_MENU),
  helpImage(Stock::HELP, ICON_SIZE_MENU),
  showMenuBarItem("_Menu Bar", true),
  showButtonBarItem("_Button Bar", true),
  showGraphTabsItem("Graph _Tabs", true),
  showStatusBarItem("_Status Bar", true),
  showGraphConfigItem("_Graph Configure", true),
  showPlotListerItem("_Plot Lists", true),
  deleteFrameMenuItem(closeImage, "Delete Frame")
{
  mainWindow = mainWindow_in;
  
  items().push_back(Menu_Helpers::MenuElem("_File", fileMenu));
  //items().push_back(Menu_Helpers::MenuElem("_Edit", editMenu));
  items().push_back(Menu_Helpers::MenuElem("_View", viewMenu));
  items().push_back(Menu_Helpers::MenuElem("_Help", helpMenu));


  { // File menu
    Menu::MenuList &menuList = fileMenu.items();

    menuList.
      push_back(Menu_Helpers::
                ImageMenuElem("_Open File ...", openImage, 
                              SigC::slot(*app, &App::openDialog)));
    
    menuList.
      push_back(Menu_Helpers::
                ImageMenuElem("New Graph _Tab", newImage, 
                              SigC::slot(*mainWindow,
                                         &MainWindow::makeNewGraphTab)));


    Glib::RefPtr<Gdk::Pixbuf> pix =
      Gdk::Pixbuf::create_from_xpm_data(newFrame16x16);
    newFrameImage.set(pix);

    menuList.
      push_back(Menu_Helpers::
                ImageMenuElem("New _Frame (Empty)", newFrameImage, 
                              SigC::slot(*app,
                                         &App::createMainWindow)));

    
    pix = Gdk::Pixbuf::create_from_xpm_data(copyFrame16x16);
    copyFrameImage.set(pix);

    menuList.
      push_back(Menu_Helpers::
                ImageMenuElem("New Frame (_Copy This)", copyFrameImage, 
                              SigC::slot(*app,
                                         &App::copyCurrentMainWindow)));

    
    pix = Gdk::Pixbuf::create_from_xpm_data(closeFrame16x16);
    closeImage.set(pix);
  
    menuList.push_back(deleteFrameMenuItem);
    deleteFrameMenuItem.signal_activate().
      connect(SigC::slot(*mainWindow, &MainWindow::deleteLater));
    closeImage.show();
    deleteFrameMenuItem.show();
    if(app->size() < 1)
      // app->size() in increased after the MainWindow is created.
      deleteFrameMenuItem.set_sensitive(false);


    pix = Gdk::Pixbuf::create_from_xpm_data(saveImage16x16);
    saveImage.set(pix);

    menuList.
      push_back(Menu_Helpers::
                ImageMenuElem("_Save PNG Image File ...", saveImage, 
                              SigC::slot(*mainWindow,
                                         &MainWindow::savePNGFile)));
    
    menuList.push_back(Menu_Helpers::SeparatorElem());


    menuList.
      push_back(Menu_Helpers::ImageMenuElem("_Quit",
                                            Gtk::Menu::AccelKey("<control>q"),
                                            quitImage,
                                            SigC::slot(*app,
                                                       &App::quit)));
    menuList.push_back(Menu_Helpers::SeparatorElem());
    menuList.push_back(Menu_Helpers::SeparatorElem());

    
    //Glib::RefPtr<AccelGroup> accel = fileMenu.get_accel_group();
    
    // Initializes menu accelerators.  So <control>q will work even if
    // the menu bar is not showing.
    fileMenu.accelerate(*mainWindow);
  }

  { // Edit menu

  }
  
  { // View menu
    Menu::MenuList &menuList = viewMenu.items();

    menuList.push_back(showMenuBarItem);
    showMenuBarItem.show();
    if(opShowMenuBar)
      showMenuBarItem.set_active();
    showMenuBarItem.signal_activate().
      connect(SigC::slot(*this, &MainMenuBar::on_showMenuBarItem));

    
    menuList.push_back(showButtonBarItem);
    showButtonBarItem.show();
    if(opShowButtons)
      showButtonBarItem.set_active();
    showButtonBarItem.signal_activate().
      connect(SigC::slot(*this, &MainMenuBar::on_showButtonBarItem));

    
    menuList.push_back(showGraphTabsItem);
    showGraphTabsItem.show();
    if(opShowGraphTabs)
      showGraphTabsItem.set_active();
    showGraphTabsItem.signal_activate().
      connect(SigC::slot(*this, &MainMenuBar::on_showGraphTabsItem));

    
    menuList.push_back(showStatusBarItem);
    showStatusBarItem.show();
    if(opShowStatusBar)
      showStatusBarItem.set_active();
    showStatusBarItem.signal_activate().
      connect(SigC::slot(*this, &MainMenuBar::on_showStatusBarItem));

    
    menuList.push_back(Menu_Helpers::SeparatorElem());

    
    menuList.push_back(showGraphConfigItem);
    showGraphConfigItem.show();
    if(opShowGraphConfig)
      showGraphConfigItem.set_active();
    showGraphConfigItem.signal_activate().
      connect(SigC::slot(*this, &MainMenuBar::on_showGraphConfigItem));

    menuList.push_back(showPlotListerItem);
    showPlotListerItem.show();
    showPlotListerItem.set_active(false);
    showPlotListerItem.signal_activate().
      connect(SigC::slot(*this, &MainMenuBar::on_showPlotListerItem));
    
  }

  { // Help menu
    Menu::MenuList &menuList = helpMenu.items();

    Glib::RefPtr<Gdk::Pixbuf> pix =
      Gdk::Pixbuf::create_from_xpm_data(about_browser16x16);
    aboutImage.set(pix);

    
    menuList.push_back(Menu_Helpers::
                       ImageMenuElem("_About", aboutImage,
                                SigC::slot(*mainWindow, &MainWindow::on_about)));
    menuList.push_back(Menu_Helpers::
                       ImageMenuElem("_Help", helpImage,
                                SigC::slot(*mainWindow, &MainWindow::on_help)));
  }
  
  signal_show().
    connect(SigC::slot(*this, &MainMenuBar::checkMenuBarState));
  signal_hide().
    connect(SigC::slot(*this, &MainMenuBar::checkMenuBarState));
  
  mainWindow->statusBar.signal_show().
    connect(SigC::slot(*this, &MainMenuBar::checkStatusBarState));
  mainWindow->statusBar.signal_hide().
    connect(SigC::slot(*this, &MainMenuBar::checkStatusBarState));
}

MainMenuBar::~MainMenuBar(void)
{
  //delete aboutImage;
  //delete newFrameImage;
}

// These 3 slots just keep the check menu items state in sync with the
// corresponding widget.
void MainMenuBar::checkMenuBarState(void) // slot
{
  // Check and fix the showMenuBarItem
  if(is_visible() && !showMenuBarItem.get_active())
    showMenuBarItem.set_active(true);
  else if(!is_visible() && showMenuBarItem.get_active())
    showMenuBarItem.set_active(false);
}

void MainMenuBar::checkButtonBarState(void) // slot
{
  // Check and fix the showButtonBarItem
  if(mainWindow->buttonBar.is_visible() && !showButtonBarItem.get_active())
    showButtonBarItem.set_active(true);
  else if(!mainWindow->buttonBar.is_visible() && showButtonBarItem.get_active())
    showButtonBarItem.set_active(false);
}

void MainMenuBar::checkGraphConfigState(void) // slot
{
  // Check and fix the showGraphConfigItem
  if(mainWindow->graphConfig && mainWindow->graphConfig->is_visible()
     && !showGraphConfigItem.get_active())
  {
    showGraphConfigItem.set_active(true);
  }
  else if((!mainWindow->graphConfig ||
           (mainWindow->graphConfig && !mainWindow->graphConfig->is_visible()))
          && showGraphConfigItem.get_active())
  {
    showGraphConfigItem.set_active(false);
  }
}

void MainMenuBar::checkPlotListerState(void) // slot
{
  // Check and fix the showPlotListerItem
  if(mainWindow->plotLister && mainWindow->plotLister->is_visible()
     && !showPlotListerItem.get_active())
  {
    showPlotListerItem.set_active(true);
  }
  else if((!mainWindow->plotLister ||
           (mainWindow->plotLister && !mainWindow->plotLister->is_visible()))
          && showPlotListerItem.get_active())
  {
    showPlotListerItem.set_active(false);
  }
}

void MainMenuBar::checkGraphTabsState(bool showing) // slot
{
  if(showing && !showGraphTabsItem.get_active())
    showGraphTabsItem.set_active(true);
  else if(!showing && showGraphTabsItem.get_active())
    showGraphTabsItem.set_active(false);
}

void MainMenuBar::checkStatusBarState(void) // slot
{
  // Check and fix the showStatusBarItem
  if(mainWindow->statusBar.is_visible() && !showStatusBarItem.get_active())
    showStatusBarItem.set_active(true);
  else if(!mainWindow->statusBar.is_visible() && showStatusBarItem.get_active())
    showStatusBarItem.set_active(false);
}


void MainMenuBar::on_showMenuBarItem(void) // slot
{
  if(showMenuBarItem.get_active())
    // This won't happen much.
    show();
  else
    hide();
}

void MainMenuBar::on_showButtonBarItem(void) // slot
{
  if(showButtonBarItem.get_active())
    mainWindow->buttonBar.show();
  else
    mainWindow->buttonBar.hide();
}

void MainMenuBar::on_showGraphTabsItem(void) // slot
{
  if(showGraphTabsItem.get_active() &&
     !mainWindow->graphsNotebook.get_show_tabs())
    mainWindow->graphsNotebook.set_show_tabs(true);
  else if(!showGraphTabsItem.get_active() &&
          mainWindow->graphsNotebook.get_show_tabs())
    mainWindow->graphsNotebook.set_show_tabs(false);
}

void MainMenuBar::on_showStatusBarItem(void)
{
  if(showStatusBarItem.get_active())
    mainWindow->statusBar.show();
  else
    mainWindow->statusBar.hide();
}



void MainMenuBar::on_showGraphConfigItem(void) // slot
{
  if(showGraphConfigItem.get_active() &&
     (!mainWindow->graphConfig || !mainWindow->graphConfig->is_visible()))
  {
    mainWindow->showGraphConfig();
  }
  else if(!showGraphConfigItem.get_active() &&
          (mainWindow->graphConfig && mainWindow->graphConfig->is_visible()))
  {
    mainWindow->graphConfig->hide();
  }
}

void MainMenuBar::on_showPlotListerItem(void) // slot
{
  if(showPlotListerItem.get_active() &&
     (!mainWindow->plotLister || !mainWindow->plotLister->is_visible()))
  {
    mainWindow->showPlotLister();
  }
  else if(!showPlotListerItem.get_active() &&
          (mainWindow->plotLister && mainWindow->plotLister->is_visible()))
  {
    mainWindow->plotLister->hide();
  }
}

