/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */
#include "config.h"

#include <string.h>
#include <stdlib.h>
#include <list>

#ifdef QP_ARCH_DARWIN
# include <limits.h>
# include <float.h>
#else
# include <values.h>
#endif

#include <gtkmm.h>
#ifdef USE_LIBSNDFILE
#  include <sndfile.h>
#endif

using namespace Gtk;
#include "value_t.h"
#include "Field.h"
#include "FieldReader.h"
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

FieldReader::FieldReader(Field *f) :
  Field(f->source)
{
  field = f;
  next = NULL;
  
  // add this object to the list of FieldReaders in the object field.
  if(field->fieldReader)
  {
    FieldReader *f = field->fieldReader;
    while(f->next) f = f->next;
    // f is the last in the list.
    f->next = this;
  }
  else // this is the first in the list
    field->fieldReader = this;

  dequeuer = field->makeDequeuer(field->numberOfValues());
}

FieldReader::~FieldReader(void)
{
  // Remove this from the Field::fieldReader list.
  FieldReader *prev=NULL, *f=field->fieldReader;
  while(f)
  {
    if(this == f)
    {
      if(prev)
        prev->next = next;
      else // this == field->fieldReader
        field->fieldReader = next;
      break;
    }

    prev = f;
    f = f->next;
  }

  field->destroyDequeuer(dequeuer);
}

void *FieldReader::makeDequeuer(count_t NumberOfValues)
{
  return field->makeDequeuer(NumberOfValues);
}

void FieldReader::destroyDequeuer(void *dequeuer)
{
  field->destroyDequeuer(dequeuer);
}
