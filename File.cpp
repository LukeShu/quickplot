/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>

#ifdef QP_ARCH_DARWIN
# include <limits.h>
# include <float.h>
#else
# include <values.h>
#endif

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

#include "MainMenuBar.h"
#include "ButtonBar.h"
#include "StatusBar.h"
#include "MainWindow.h"
#include "App.h"

#include "Globel.h"

#include "Source.h"
#include "FileList.h"
#include "File.h"
#include "errorStr.h"

#ifdef MINGW  /* MinGW on windoz */
#  define DIR_CHAR '\\'
#else
#  define DIR_CHAR '/'
#endif

File::File(const char *filename)
{
  FileList fileList;
  fileList.setFileName(filename);
  init(&fileList);
}

File::File(const FileList *fileList)
{
  init(fileList);
}
 

void File::init(const FileList *fileList)
{
  isValid=false;
  char *filename = fileList->getFileName();

  bool isStdin = (filename)? false: true;
  
  
  // Check for errors in filename.
  if(filename && !filename[0])
    {
      // zero length file name.
      sprintf(errorStr,"File name \"%s\" is invalid.", filename);
      return; // failed.
    }

  if(filename)
  {
    size_t len=strlen(filename);
    // Check for errors in filename again.
    if(filename[len-1] == DIR_CHAR)
    {
      // The filename end with a DIR_CHAR (/).
      snprintf(errorStr, ERRORSTR_LENGTH,
               "Invalid file name \"%s\".", filename);
      return; // failed.
    }
  }
  
  // Set the fileName
  if(filename) // It's not stdin.
    fileName = strdup(filename);
  else // It's stdin.
    fileName = strdup(STDIN_FILENAME);

  {
    size_t len=strlen(fileName);
    size_t  i;
    for(i=len-1; i>0 && fileName[i] != DIR_CHAR; i--);
    if(i>0) i++;
    baseFileName = strdup(&fileName[i]);
  }

  error = 0;

#ifdef USE_LIBSNDFILE

  // readSndFile() or readASCIIFile() may set error.
  if(!isStdin && readSndFile(fileList))
    isValid = !error;
  else

#endif // #ifdef USE_LIBSNDFILE

  if(readASCIIFile(isStdin ? stdin : NULL, fileList))
    isValid = !error;
  else
    isValid = false;


  if(isValid)
  {
    // signal that the files that are loaded changed.
    m_signal_addedSource.emit(this);
    // Add a File->"Close fileName" to the MainMenu.  Base class
    // Source::~Source() will remove it automatically.
    addCloseMenus(fileName);
  }
}


//File::~File(void)
//{
// Source::~Source() will remove all the fields.
//}
