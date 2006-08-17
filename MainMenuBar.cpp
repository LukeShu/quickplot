/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */
#include "config.h"

#include <list>

#ifdef QP_ARCH_DARWIN
# include <limits.h>
# include <float.h>
#else
# include <values.h>
#endif

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
#include "imgAbout_browser.xpm"
#include "imgNewFrame.xpm"
#include "imgCloseFrame.xpm"
#include "imgCopyFrame.xpm"
#include "imgSaveImage.xpm"

// accel_key 	GDK keyval of the accelerator. are found in gdk/gdkkeysyms.h

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
  deleteFrameMenuItem(closeImage, "_Delete Frame", true)
{
  mainWindow = mainWindow_in;
  
  items().push_back(Menu_Helpers::MenuElem("File", fileMenu));
  items().push_back(Menu_Helpers::MenuElem("View", viewMenu));
  items().push_back(Menu_Helpers::MenuElem("Help", helpMenu));


  { // File menu
    Menu::MenuList &menuList = fileMenu.items();

    menuList.
      push_back(Menu_Helpers::
                ImageMenuElem("_Open File ...", openImage, 
                              sigc::mem_fun(*app, &App::openDialog)));
    // make it so we activate with <alt>m, <control>m, and etc.
    addAccelKey(&(menuList.back()), GDK_o);

    
    menuList.
      push_back(Menu_Helpers::
                ImageMenuElem("_New Graph Tab", newImage, 
                              sigc::mem_fun(*mainWindow,
                                         &MainWindow::makeNewGraphWithGraphConfig)));
    // make it so we activate with <alt>m, <control>m, and etc.
    addAccelKey(&(menuList.back()), GDK_n);


    Glib::RefPtr<Gdk::Pixbuf> pix =
      Gdk::Pixbuf::create_from_xpm_data(imgNewFrame);
    newFrameImage.set(pix);

    menuList.
      push_back(Menu_Helpers::
                ImageMenuElem("New _Frame (Empty)", newFrameImage, 
                              sigc::mem_fun(*app,
                                         &App::createMainWindow)));
    // make it so we activate with <alt>m, <control>m, and etc.
    addAccelKey(&(menuList.back()), GDK_f);

    
    pix = Gdk::Pixbuf::create_from_xpm_data(imgCopyFrame);
    copyFrameImage.set(pix);

    menuList.
      push_back(Menu_Helpers::
                ImageMenuElem("New Frame (_Copy This)", copyFrameImage, 
                              sigc::mem_fun(*app,
                                         &App::copyCurrentMainWindow)));
    // make it so we activate with <alt>m, <control>m, and etc.
    addAccelKey(&(menuList.back()), GDK_c);

    
    pix = Gdk::Pixbuf::create_from_xpm_data(imgCloseFrame);
    closeImage.set(pix);
  
    menuList.push_back(deleteFrameMenuItem);
    deleteFrameMenuItem.signal_activate().
      connect(sigc::mem_fun(*mainWindow, &MainWindow::deleteLater));
    closeImage.show();
    deleteFrameMenuItem.show();
    if(app->size() < 1)
      deleteFrameMenuItem.set_sensitive(false);
    // make it so we activate with <alt>m, <control>m, and etc.
    addAccelKey(&(menuList.back()), GDK_d);
    addAccelKey(&(menuList.back()), GDK_Escape);


    pix = Gdk::Pixbuf::create_from_xpm_data(imgSaveImage);
    saveImage.set(pix);

    menuList.
      push_back(Menu_Helpers::
                ImageMenuElem("Save PNG _Image File ...", saveImage, 
                              sigc::mem_fun(*mainWindow,
                                         &MainWindow::savePNGFile)));
    // make it so we activate with <alt>m, <control>m, and etc.
    addAccelKey(&(menuList.back()), GDK_i);

    
    menuList.push_back(Menu_Helpers::SeparatorElem());


    menuList.
      push_back(Menu_Helpers::ImageMenuElem("_Quit",
                                            quitImage,
                                            sigc::mem_fun(*app,
                                                       &App::quit)));
    // make it so we activate with <alt>m, <control>m, and etc.
    addAccelKey(&(menuList.back()), GDK_q);
    
    menuList.push_back(Menu_Helpers::SeparatorElem());
    menuList.push_back(Menu_Helpers::SeparatorElem());

    
    //Glib::RefPtr<AccelGroup> accel = fileMenu.get_accel_group();
    
    // Initializes menu accelerators.  So <control>q will work even if
    // the menu bar is not showing.
    fileMenu.accelerate(*mainWindow);
  }
  
  { // View menu
    Menu::MenuList &menuList = viewMenu.items();

    menuList.push_back(showMenuBarItem);
    showMenuBarItem.show();
    if(opShowMenuBar)
      showMenuBarItem.set_active();
    showMenuBarItem.signal_activate().
      connect(sigc::mem_fun(*this, &MainMenuBar::on_showMenuBarItem));
    // make it so we activate with <alt>m, <control>m, and etc.    
    addAccelKey(&showMenuBarItem, GDK_m);
    
    
    menuList.push_back(showButtonBarItem);
    showButtonBarItem.show();
    if(opShowButtons)
      showButtonBarItem.set_active();
    showButtonBarItem.signal_activate().
      connect(sigc::mem_fun(*this, &MainMenuBar::on_showButtonBarItem));
     // make it so we activate with <alt>m, <control>m, and etc.    
     addAccelKey(&showButtonBarItem, GDK_b);
    
    
    menuList.push_back(showGraphTabsItem);
    showGraphTabsItem.show();
    if(opShowGraphTabs)
      showGraphTabsItem.set_active();
    showGraphTabsItem.signal_activate().
      connect(sigc::mem_fun(*this, &MainMenuBar::on_showGraphTabsItem));
    // make it so we activate with <alt>m, <control>m, and etc.    
    addAccelKey(&showGraphTabsItem, GDK_t);

    
    menuList.push_back(showStatusBarItem);
    showStatusBarItem.show();
    if(opShowStatusBar)
      showStatusBarItem.set_active();
    showStatusBarItem.signal_activate().
      connect(sigc::mem_fun(*this, &MainMenuBar::on_showStatusBarItem));
    // make it so we activate with <alt>m, <control>m, and etc.    
    addAccelKey(&showStatusBarItem, GDK_s);

    
    menuList.push_back(Menu_Helpers::SeparatorElem());

    
    menuList.push_back(showGraphConfigItem);
    showGraphConfigItem.show();
    if(opShowGraphConfig)
      showGraphConfigItem.set_active();
    showGraphConfigItem.signal_activate().
      connect(sigc::mem_fun(*this, &MainMenuBar::on_showGraphConfigItem));
    // make it so we activate with <alt>m, <control>m, and etc.    
    addAccelKey(&showGraphConfigItem, GDK_g);


    menuList.push_back(showPlotListerItem);
    showPlotListerItem.show();
    showPlotListerItem.set_active(false);
    showPlotListerItem.signal_activate().
      connect(sigc::mem_fun(*this, &MainMenuBar::on_showPlotListerItem));
    // make it so we activate with <alt>m, <control>m, and etc.    
    addAccelKey(&showPlotListerItem, GDK_p);
    
    viewMenu.accelerate(*mainWindow);
  }

  { // Help menu
    Menu::MenuList &menuList = helpMenu.items();

    Glib::RefPtr<Gdk::Pixbuf> pix =
      Gdk::Pixbuf::create_from_xpm_data(imgAbout_browser);
    aboutImage.set(pix);

    
    menuList.push_back(Menu_Helpers::
                       ImageMenuElem("_About", aboutImage,
                                sigc::mem_fun(*mainWindow,
                                           &MainWindow::on_about)));
    // make it so we activate with <alt>m, <control>m, and etc.
    addAccelKey(&(menuList.back()), GDK_a);

    menuList.push_back(Menu_Helpers::
                       ImageMenuElem("_Help", helpImage,
                                sigc::mem_fun(*mainWindow,
                                           &MainWindow::on_help)));
    // make it so we activate with <alt>m, <control>m, and etc.
    addAccelKey(&(menuList.back()), GDK_h);

  }
  
  signal_show().
    connect(sigc::mem_fun(*this, &MainMenuBar::checkMenuBarState));
  signal_hide().
    connect(sigc::mem_fun(*this, &MainMenuBar::checkMenuBarState));
  
  mainWindow->statusBar.signal_show().
    connect(sigc::mem_fun(*this, &MainMenuBar::checkStatusBarState));
  mainWindow->statusBar.signal_hide().
    connect(sigc::mem_fun(*this, &MainMenuBar::checkStatusBarState));
}

void MainMenuBar::addAccelKey(Widget *widget, gint key)
{
  // this one makes it happen with just the key alone.
  widget->add_accelerator("activate", mainWindow->get_accel_group(),
                         key, Gdk::LOCK_MASK, ACCEL_MASK);

#if 0 // the MainWindow takes care of this now.
  widget->add_accelerator("activate", mainWindow->get_accel_group(),
                         key, Gdk::CONTROL_MASK, ACCEL_MASK);
  widget->add_accelerator("activate", mainWindow->get_accel_group(),
                         key, Gdk::SHIFT_MASK, ACCEL_MASK);
  widget->add_accelerator("activate", mainWindow->get_accel_group(),
                         key, Gdk::MOD1_MASK, ACCEL_MASK);
  widget->add_accelerator("activate", mainWindow->get_accel_group(),
                         key, Gdk::MOD2_MASK, ACCEL_MASK);
  widget->add_accelerator("activate", mainWindow->get_accel_group(),
                         key, Gdk::MOD3_MASK, ACCEL_MASK);
  widget->add_accelerator("activate", mainWindow->get_accel_group(),
                         key, Gdk::MOD4_MASK, ACCEL_MASK);
  widget->add_accelerator("activate", mainWindow->get_accel_group(),
                         key, Gdk::MOD5_MASK, ACCEL_MASK);
#endif
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

