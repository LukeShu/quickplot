/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */
#include "config.h"

#ifdef USE_LIBSNDFILE

#include <iostream>
#include <values.h>
#include <list>
#include <iomanip>
#include <sndfile.h>

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
#include "errorStr.h"
#include "ArrayField.h"
#include "LinearField.h"


#ifdef  TYPE_INT
# define TYPE int
# define readSndTYPE readSndInt
# define readFunc sf_readf_int
#endif

#ifdef  TYPE_FLOAT
# define TYPE float
# define readSndTYPE readSndFloat
# define readFunc sf_readf_float
#endif

#ifdef  TYPE_DOUBLE
# define TYPE double
# define readSndTYPE readSndDouble
# define readFunc sf_readf_double
#endif

// The default case is TYPE short
#if (!defined TYPE_INT &&\
     !defined TYPE_FLOAT &&\
     !defined TYPE_DOUBLE)
# define TYPE short
# define readSndTYPE readSndShort
# define readFunc sf_readf_short
#endif


// C++ Templates will not make methods with templates without making
// the whole class a template. I did not want to make the class File a
// template, that would be lots more work than this.  So I generate
// the code from this file for the following 4 methods:
  
// by default               File::readSndShort()
// by defining TYPE_INT     File::readSndInt()
// by defining TYPE_FLOAT   File::readSndFloat()
// by defining TYPE_DOUBLE  File::readSndDouble()


void File::
readSndTYPE(::SNDFILE *sndfile, int samplerate,
            count_t numberOfValues, int channels,
            const FileList *fileList)
{
  ArrayField<TYPE> **field =
    static_cast<ArrayField<TYPE> **>
    (malloc(sizeof(ArrayField<TYPE> *)*channels));
  TYPE *data = static_cast<TYPE *>(malloc(sizeof(TYPE)*channels));

  int i;
  count_t j=0;
  error = 0;
  // get the fields
  for(i=0;i<channels;i++)
  {
    field[i] = new ArrayField<TYPE>(this, numberOfValues);
    if(error)
    {
      int k;
      for(k=0;k<=i;k++)
        delete field[k];
      goto cleanup;
    }
  }

  {
    // add a linear field and add fields to the list
    Field *lField = new LinearField(this, 1.0/((value_t) samplerate));
    // Make this LinearField first in the list.
    remove(lField);
    push_front(lField);
    lField->setName("seconds");
    lField->setLabel("time");
  }
  
  for(i=0;i<channels;i++)
  {
    char str[16];
    sprintf(str, "%d", i+1);
    field[i]->setName(str);
    field[i]->setLabel("channel");
  }

  // read the data and put it in the field
  for(;j<numberOfValues;j++)
  {
    if(1 != readFunc(sndfile, data, 1))
    {
      error = 1;
      snprintf(errorStr, ERRORSTR_LENGTH,
               "error using libsndfile: %s", sf_strerror(sndfile));
      break;
    }
    
    for(i=0;i<channels;i++)
      field[i]->write(data[i]);
  }

  cleanup:
  
  free(data);
  free(field);
}

#endif // #ifdef USE_LIBSNDFILE
