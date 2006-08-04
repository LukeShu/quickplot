/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */

class GraphConfig;
class MainWindow;

// There can only be one App per process.

class App : public Main, public std::list<MainWindow *>
{
public:

  bool isInvalid;

  void quit(void);


  // create a main window and add it to the list.
  void createMainWindow(void);
  void copyCurrentMainWindow(void);
  void destroyMainWindow(MainWindow *mainWindow);


 private:

  // parseArgs1() gets options that need to be parsed before others
  // like --verbose, options that cause this program to exit like
  // --help, and options that change the way the main window looks
  // like --no-buttons.  parseArgs1() returns non-zero on error.
  int parseArgs1(int argc, char **argv);

  // parseArgs2() returns non-zero on error.  parseArgs2 parses stuff
  // to open files and more.  parseArgs2 needs to be called after
  // parseArgs1() and createMainWindow().
  int parseArgs2(int argc, char **argv);

  // used by createMainWindow() and copyCurrentMainWindow()
  void _createMainWindow(bool makeFirstGraph);

  FileSelection *fileSelection;
  
 public:

  App(int *argv, char ***argv);
  virtual ~App(void);


  void openDialog(void); // slot
  void openFile(const char *filename);

};

extern App *app;
extern MainWindow *currentMainWindow;
