/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */

class MainWindow;

class MainMenuBar : public MenuBar
{
 public:
  
  MainMenuBar(MainWindow *mainWindow_in);
  ~MainMenuBar(void);


  // getFileMenu() is used by Source when a file is opened a Close
  // file menu item is added.
  inline Menu &getFileMenu(void) { return fileMenu; }


  // These slots do the show or hide of the corresponding widget.
  void on_showMenuBarItem(void); // slot
  void on_showButtonBarItem(void); // slot
  void on_showGraphTabsItem(void); // slot
  void on_showStatusBarItem(void); // slot
  void on_showGraphConfigItem(void); // slot
  void on_showPlotListerItem(void); // slot


  // These"check" slots just keep the check menu items state in sync
  // with the corresponding widget.
  void checkMenuBarState(void); // slot
  void checkButtonBarState(void); // slot
  void checkGraphTabsState(bool showing); // slot
  void checkStatusBarState(void); // slot
  void checkGraphConfigState(void); // slot
  void checkPlotListerState(void); // slot
  
private:

  void addAccelKey(Widget *widget, gint key);
  
  Menu fileMenu, /* editMenu, */ viewMenu, helpMenu;
  
  // File Menu
  Image openImage, newImage, newFrameImage, copyFrameImage,
    closeImage, quitImage, aboutImage, helpImage, saveImage;

  // View Menu
  Gtk::CheckMenuItem showMenuBarItem, showButtonBarItem,
    showGraphTabsItem, showStatusBarItem, showGraphConfigItem,
    showPlotListerItem;

public:
  
  ImageMenuItem deleteFrameMenuItem;
  
private:

  MainWindow *mainWindow;
};
