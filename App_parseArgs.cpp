/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <list>
#include <iomanip>

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
#include "Globel.h"
#include "PlotList.h"
#include "config.h"

// from help_html.cpp
extern const char *help_html;

// from about_html.cpp
extern const char *about_html;

// from usage.cpp
extern int usage(void);

// from getOpt.cpp
extern char *getOpt(const char *shorT, const char *lonG,
		    int argc, const char **argv, int *i);

enum ReadStdin { NO=0, YES=1, CHECK=2 };
static ReadStdin readStdin = CHECK;


// from file: help_html.cpp which was generated from help.html.in
extern const char *help_html;

// from file: about_html.cpp which was generated from about.html.in
extern const char *about_html;


// From file: launchBrowser.cpp
extern int sLaunchBrowser(const char *doc, const char *suffix);

void showAbout(void)
{
  // first we attack with a browser
  sLaunchBrowser(about_html, "about.html");
  
  // next we spew to stdout.
  printf("%s version %s : released on %s\n", PACKAGE_NAME, VERSION, RELEASE_DATE);
  printf("The %s home-page is at %s\n", PACKAGE_NAME, HOMEPAGE);
  printf("Send BUG reports to: %s\n", PACKAGE_BUGREPORT);
  printf("This was configured on: %s\n", CONFIGURE_DATE);
  printf("%s is free software, distributed under the "
         "terms of the\nGNU General Public License (version 2 or higher).\n",
         PACKAGE_NAME);
  exit(1); // exit with an error.
}

void showHelp(void)
{
  // first we attack with a browser
  sLaunchBrowser(help_html, "help.html");
  // next we spew to stdout.
  printf("%s is a fast interactive 2D plotter.\n\n", PACKAGE_NAME);
  printf("%s uses html files for its reference documentation\n", PACKAGE_NAME);
  printf("If you're not seeing a browser now with the %s Help\ndocument loaded "
         "than you may not be using a windowing system\nor something else went "
         "wrong.\n\n", PACKAGE_NAME);
  printf("The %s HTML help document can be printed to standard output by\n"
         "running `quickplot --print-help'.\n\n", PACKAGE_NAME);
  printf("The %s home-page is at %s\n", PACKAGE_NAME, HOMEPAGE);
  exit(1); // exit with an error.
}


// first get options that need to be parsed before others like
// --verbose, options that cause this program to exit like --help, and
// options that change the way the main window looks like --no-buttons
// and --same-scale (which can change the first Graph).
int App::parseArgs1(int argc, char **argv)
{  
  int i;
  for(i=1;i<argc;)
  {
    char *s;
    
    if(!strcmp("--about",argv[i]) || !strcmp("-a",argv[i]) )
    {
      showAbout();
    }
    else if((s = getOpt("-c", "--canvas-color", argc, (const char **) argv, &i)))
    {
      Gdk::Color color(s);

      ColorGen::backgroundColor[0] = color.get_red();
      ColorGen::backgroundColor[1] = color.get_green();
      ColorGen::backgroundColor[2] = color.get_blue();
    }
    else if((s = getOpt("-C", "--grid-color", argc,
                        (const char **) argv, &i)))
    {
      Gdk::Color color(s);

      ColorGen::gridColor[0] = color.get_red();
      ColorGen::gridColor[1] = color.get_green();
      ColorGen::gridColor[2] = color.get_blue();
    }
    else if(!strcmp("--different-scale",argv[i]) || !strcmp("-d",argv[i]))
    {
      opSameScale = false;
      opAutoSameScale = false;
      i++;
    }
    else if((s = getOpt("-geometry", "--geometry", argc,
                        (const char **) argv, &i)))
    {
      opGeometry = s;
    }    
    else if(!strcmp("--no-buttons",argv[i]) || !strcmp("-B",argv[i]))
    {
      opShowButtons = false;
      i++;
    }
    else if((s = getOpt("-g", "--grid-line-width",
                        argc, (const char **) argv, &i)))
    {
      errno = 0;
      char *ptr = s;
      opGridLineWidth = strtol(s, &ptr, 10);
      if(ptr==s || errno || opGridLineWidth<0 || opGridLineWidth>MAX_GRID_LINE_WIDTH)
      {
        if(!opSilent)
          opSpew << "quickplot ERROR: bad option -g or --grid-line-width"
                 << std::endl;
        if(!opSilent && opGridLineWidth>MAX_GRID_LINE_WIDTH)
          opSpew << "the grid line width may not be larger than "
                 << MAX_GRID_LINE_WIDTH << " ." << std::endl;
        return -1; // error
      }
    }
    else if(!strcmp("--help",argv[i]) || !strcmp("-h",argv[i]))
    {
      showHelp();
    }
    else if((s = getOpt("-I", "--line-width", argc, 
                        (const char **) argv, &i)))
    {
      errno = 0;
      char *ptr = s;
      opLineWidth = strtol(s, &ptr, 10);
      if(ptr==s || errno || opLineWidth<0 || opLineWidth>MAXMAX_PLOT_LINE_WIDTH)
      {
        if(!opSilent)
          opSpew << "quickplot ERROR: bad option --line-width or -I"
                 << std::endl;
        if(!opSilent && opLineWidth>MAXMAX_PLOT_LINE_WIDTH)
          opSpew << "the line width may not be larger than "
                 << MAXMAX_PLOT_LINE_WIDTH << " ." << std::endl;
        return -1; // error
      }
      opUserSetLineOrPointOption = true;
    }
    else if(!strcmp("--no-grid",argv[i]) || !strcmp("-G",argv[i]))
    {
      opShowAutoGrid = false;
      i++;
    }
    else if(!strcmp("--no-lines",argv[i]) || !strcmp("-i",argv[i]))
    {
      opShowLines = false;
      opUserSetLineOrPointOption = true;
      i++;
    }
    else if(!strcmp("--no-points",argv[i]) || !strcmp("-o",argv[i]))
    {
      opShowPoints = false;
      opUserSetLineOrPointOption = true;
      i++;
    }
    else if(!strcmp("--no-statusbar",argv[i]))
    {
      opShowStatusBar = false;
      i++;
    }
    else if(!strcmp("--no-tabs",argv[i]))
    {
      opShowGraphTabs = false;
      i++;
    }
    else if((s = getOpt("-O", "--point-size", argc,
                        (const char **) argv, &i)))
    {
      errno = 0;
      char *ptr = s;
      opPointSize = strtol(s, &ptr, 10);
      if(ptr==s || errno || opPointSize<0 || opPointSize>MAXMAX_POINT_SIZE)
      {
        if(!opSilent)
          opSpew << "quickplot ERROR: bad option "
            "-O or --point-size\n" << std::endl;
        return -1; // error
      }
      opUserSetLineOrPointOption = true;
    }
    else if(!strcmp("--same-scale",argv[i]) || !strcmp("-s",argv[i]))
    {
      opSameScale = true;
      i++;
    }
    else if(!strcmp("--verbose",argv[i]) || !strcmp("-v",argv[i]))
    {
      opVerbose = true;
      i++;
    }
    else if(!strcmp("--version",argv[i]) || !strcmp("-V",argv[i]))
    {
      printf("%s\n", VERSION);
      return -1;
    }
    else if(!strcmp("--print-about",argv[i]))
    {
      printf("%s\n", about_html);
      exit(0); // must return success so make will continue
    }
    else if(!strcmp("--print-help",argv[i]))
    {
      printf("%s\n", help_html);
      exit(0); // must return success so make will continue
    }
    else if(!strcmp("--no-menubar",argv[i]) || !strcmp("-M",argv[i]))
    {
      opShowMenuBar = false;
      i++;
    }
    // looking for all other single '-' options not handled above.
    else if(strlen(argv[i]) > (size_t) 2 &&
            argv[i][0] == '-' &&
            ( 
              argv[i][1] == 'a' ||
              argv[i][1] == 'B' ||
              argv[i][1] == 'd' ||
              argv[i][1] == 'G' ||
              argv[i][1] == 'h' ||
              argv[i][1] == 'i' ||
              argv[i][1] == 'L' ||
              argv[i][1] == 'M' ||
              argv[i][1] == 'N' ||
              argv[i][1] == 'o' ||
              argv[i][1] == 'P' ||
              argv[i][1] == 's' ||
              argv[i][1] == 'V' ||
              argv[i][1] == 'v'
              )
	    )
    {
      int j=1;
      for(;argv[i][j];j++)
      {
        switch (argv[i][j])
        {
          case 'a':
            showAbout();
            break;
          case 'B':
            opShowButtons = false;
            break;
          case 'd':
            opSameScale = false;
            opAutoSameScale = false;
            break;
          case 'G':
            opShowAutoGrid = false;
            break;
          case 'h':
            showHelp();
            break;
          case 'i':
            opShowLines = false;
            break;
          case 'L': // handled later.
            break;
          case 'M':
            opShowMenuBar = false;
            break;
          case 'N':
            readStdin = NO;
            break;
          case 'o':
            opShowPoints = false;
            break;
          case 'P':
            readStdin = YES;
            break;
          case 's':
            opSameScale = true;
            break;
          case 'V':
            printf("%s\n",VERSION);
            return -1;
            break;
          case 'v':
            opVerbose = true;
            opSilent = false;
            break;
          default:
          {
            if(!opSilent)
              opSpew << "quickplot ERROR: bad option(s) '"
                     <<  argv[i] << "'" << std::endl;
            return -1; // error
            break;
          }
        }
      }
      i++;
    }
    // skipping the processing of other options.
    else
      i++;
  }
  return 0; // success
}



// This is called after App::parseArgs1() and other things.
int App::parseArgs2(int argc, char **argv)
{

  // The first file will be stdin if we are reading stdin.
  FileList *fileList = new FileList(NULL);
  PlotList *plotList = NULL;

  int i;
  for(i=1;i<argc;)
  {
    char *s;
    
    if(!strcmp("--about",argv[i]) || !strcmp("-a",argv[i])
       ||
       (!strcmp("--different-scale",argv[i]) || !strcmp("-d",argv[i]))
       ||
       !strcmp("--help",argv[i]) || !strcmp("-h",argv[i])
       ||
       !strcmp("--no-buttons",argv[i]) || !strcmp("-B",argv[i])
       ||
       !strcmp("--no-grid",argv[i]) || !strcmp("-G",argv[i])
       ||
       !strcmp("--no-lines",argv[i]) || !strcmp("-i",argv[i])
       ||
       !strcmp("--no-points",argv[i]) || !strcmp("-o",argv[i])
       ||
       !strcmp("--no-menubar",argv[i]) || !strcmp("-M",argv[i])
       ||
       !strcmp("--no-statusbar",argv[i])
       ||
       !strcmp("--no-tabs",argv[i])
       ||
       !strcmp("--same-scale",argv[i]) || !strcmp("-s",argv[i])
       ||
       !strcmp("--verbose",argv[i]) || !strcmp("-v",argv[i])
       )
    {
      // All handled already in App::getArgs1()
      i++;
    }
    else if(
      (s = getOpt("-c", "--canvas-color", argc, (const char **) argv, &i))
      ||
      (s = getOpt("-C", "--grid-color", argc, (const char **) argv, &i))
      ||
      (s = getOpt("-geometry", "--geometry", argc, (const char **) argv, &i))
      ||
      (s = getOpt("-g", "--grid-line-width", argc, (const char **) argv, &i))
      ||
      (s = getOpt("-I", "--line-width", argc, (const char **) argv, &i))
      ||
      (s = getOpt("-O", "--point-size", argc, (const char **) argv, &i))
      )
    {
      // All handled already in App::getArgs1()
    }
    else if((s = getOpt("-b", "--label-separator", argc, (const char **) argv, &i)))
    {
      fileList->labelSeparator = s[0];
    }
    else if(!strcmp("--no-default-plots",argv[i]))
    {
      opNoDefaultPlots = true;
      i++;
    }
    else if(!strcmp("--no-pipe",argv[i]) || !strcmp("-N",argv[i]))
    {
      readStdin = NO;
      i++;
    }
    else if(!strcmp("--pipe",argv[i]) || !strcmp("-P",argv[i]))
    {
      readStdin = YES;
      i++;
    }
    else if(!strcmp("-l",argv[i]) || !strcmp("--linear-field",argv[i]))
    {
      i++;
      fileList->hasLinearField = true;
      int j=0;
      while(j<2) // check for options  --start or -r or  OR --step or -t.
        // Since both may be persent we check for either one
        // twice.
      {
        if((s = getOpt("-r", "--start", argc, (const char **) argv, &i)))
        {
          errno = 0;
          char *ptr = s;
          fileList->linearFieldStart = STRINGTOVALUE(s, &ptr);
          if(ptr==s || errno)
          {
            if(!opSilent)
              opSpew << "quickplot ERROR: bad option --start or -r" << std::endl;
            return -1; // error
          }
        }
        else if((s = getOpt("-t","--step", argc, (const char **) argv, &i)))
        {
          errno = 0;
          char *ptr = s;
          fileList->linearFieldStep = STRINGTOVALUE(s, &ptr);
          if(ptr==s || errno)
          {
            if(!opSilent)
              opSpew << "quickplot ERROR: bad option --step or -t" << std::endl;
            return -1; // error
          }
        }
        else
          break;
        j++;
      }
    }
#if 0
    else if(!strcmp("--log",argv[i]))
    {
      fileList->takeLog = true;
      i++;
    }
#endif
    else if((s = getOpt("-p", "--plot", argc, (const char **) argv, &i)))
    {
      errno = 0;
      char *ptr = s;
      int count = 0;
      plotList = new PlotList;
      for(;;count++)
      {
        s = ptr;
        int x = strtol(s, &ptr, 10);
        if(ptr==s || errno)
          break;
        s = ptr;
        int y = strtol(s, &ptr, 10);
        if(ptr==s || errno)
        {
          if(!opSilent)
            opSpew << "quickplot ERROR: bad option"
              " --plot or -p\n" << std::endl;
          return -1; // error
        }

        plotList->add(x, y);
      }
      if(count < 1)
      {
        if(!opSilent)
          opSpew << "quickplot ERROR: bad option --plot or -p\n" << std::endl;
        return -1; // error
      }
    }
    else if((s = getOpt("-n", "--number-of-plots",
                        argc, (const char **) argv, &i)))
    {
      errno = 0;
      char *ptr = s;
      opMaxNumDefaultPlots = strtol(s, &ptr, 10);
      if(ptr==s || errno)
      {
        if(!opSilent)
          opSpew << "quickplot ERROR: bad option "
            "-n or --number-of-plots\n" << std::endl;
        return -1; // error
      }
      if(opMaxNumDefaultPlots > 10000)
      {
        if(!opSilent)
        {
          opSpew << "quickplot INFO: funny option "
            "-n or --number-of-plots\n" << std::endl
                 << "How do you expect to see " << opMaxNumDefaultPlots
                 << " plots in one Graph?" <<  std::endl;
        }
        opMaxNumDefaultPlots = 10000;
      }
    }
    else if((s = getOpt("-S", "--skip-lines", 
			    argc, (const char **) argv, &i)))
    {
      errno = 0;
      char *ptr = s;
      int skip = strtol(s, &ptr, 10);
      if(ptr==s || errno || skip < 0)
      {
        if(!opSilent)
          opSpew << "quickplot ERROR: bad option -S or "
            "--skip-lines\n" << std::endl;
        return -1; // error
      }
      fileList->skipLines = skip;
    }
    else if(!strcmp("-L",argv[i]) || !strcmp("--labels",argv[i]))
    {
      fileList->readLabels = true;
      i++;
    }
    else if((s = getOpt("-b", "--label-seperator", argc, (const char **) argv, &i)))
    {
      if(s[0] == '\n' ||
         s[0] == (char) EOF ||
         s[0] == 0)
      {
        printf("quickplot ERROR:  bad option -b or "
               "--label-seperator: invalid seperator character\n");
        return -1; // error
      }
      fileList->labelSeparator = s[0];
    }
    else if(strlen(argv[i]) > (size_t) 2 &&
            argv[i][0] == '-' &&
            ( argv[i][1] == 'a' ||
              argv[i][1] == 'B' ||
              argv[i][1] == 'G' ||
              argv[i][1] == 'i' ||
              argv[i][1] == 'h' ||
              argv[i][1] == 'L' ||
              argv[i][1] == 'M' ||
              argv[i][1] == 'N' ||
              argv[i][1] == 'o' ||
              argv[i][1] == 'P' ||
              argv[i][1] == 's' ||
              argv[i][1] == 'v'
              )
            )
    {
      int j=1;
      for(;argv[i][j];j++)
      {
        switch (argv[i][j])
        {
          case 'L':
            fileList->readLabels = true;
            break;
            // all of the cases below are handled above in
            // App::parseArgs1() already.
          case 'a':
          case 'B':
          case 'G':
          case 'i':
          case 'h':
          case 'M':
          case 'N':
          case 'o':
          case 'P':
          case 's':
          case 'v':
          default:
            break;
        }
      }
      i++;
    }
    
    else // get a file name to load
    {
      fileList = new FileList(argv[i]);
      i++;
    }
  }
  
  if(readStdin == CHECK)
  { // Check for standard input
    fd_set rfds;
    struct timeval tv = { 0/* sec */, 200000 /* usec */ };
    FD_ZERO(&rfds);
    FD_SET(0, &rfds);
    
    if(select(1, &rfds, NULL, NULL, &tv) == 1)
      readStdin = YES;
  }
  
  if(readStdin == YES)
  {
    fileList = FileList::first;
    File *file = new File(fileList);
    
    if(!file->isValid)
    {
      delete file;
      return -1; //error
    }
    fclose(stdin);
  }
  
  // The first file in the list is stdin, so skip to the next one.
  if(FileList::first->next)
  {
    fileList = FileList::first->next;
    for(;fileList && fileList->getFileName();fileList = fileList->next)
    {
      File *file = new File(fileList);
      
       if(!file->isValid)
       {
         delete file;
         return -1; //error
       }
    }
  }

  // delete all the file lists.
  while(FileList::first)
    delete FileList::first;

  
  if(!(PlotList::first) && !opNoDefaultPlots)
  {
    // make the default plot list.
    int fieldCount = 0;
    std::list<Source *>::const_iterator source = sources.begin();
    for(;source != sources.end(); source++)
    {
      int numPlots = (*source)->size() - 1;
      if(numPlots > opMaxNumDefaultPlots)
        numPlots = opMaxNumDefaultPlots;

      if(numPlots > 0)
      {
        plotList = new PlotList;

        int i;
        for(i=0;i<numPlots;i++)
        {
          plotList->add(fieldCount,i+1+fieldCount);
        }
        
        fieldCount += (*source)->size();
      }
    }
  }

  
  if(PlotList::first) // is there a plot list
  {
    plotList = PlotList::first;
    for(; plotList; plotList=plotList->next)
    {
      if(plotList != PlotList::first)
      {
        currentMainWindow->makeNewGraphTab();
      }
      
      int i;
      for(i=0;i<plotList->getNumberOfPlots();i++)
      {
        int x, y;
        plotList->get(x,y);
        Field *X=NULL, *Y=NULL;
        int fieldCount = 0;
        std::list<Source *>::const_iterator source = sources.begin();
        for(;source != sources.end(); source++)
        {    
          std::list<Field *>::const_iterator field = (*source)->begin();
          for(;field != (*source)->end(); field++)
          {
            if(x == fieldCount)
              X = *field;
            if(y == fieldCount)
              Y = *field;
            fieldCount++;
            if(X && Y)
              break;
          }
          if(X && Y)
            break;
        }
        if(!X || !Y)
        {
          if(!opSilent)
            opSpew << "quickplot ERROR: bad option --plot or -p\n"
                   << std::endl;
          return -1; // error
        }
        currentMainWindow->currentGraph->createPlot(X, Y);
      }
    }
    // flip to the first page (tab) in the graph notebook.
    currentMainWindow->graphsNotebook.set_current_page(0);
  }
  
  // delete all the plot lists.
  while(PlotList::first)
    delete PlotList::first;

 
  return 0; // success
}
