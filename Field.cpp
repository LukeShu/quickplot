/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */
#include "config.h"

#include <string.h>
#include <stdlib.h>
#include <list>
#include <values.h>

#include <gtkmm.h>


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
#include "Source.h"

#include "MainMenuBar.h"
#include "ButtonBar.h"
#include "StatusBar.h"
#include "MainWindow.h"
#include "App.h"

#include "Globel.h"

static int fieldCount = 0;

Field::Field(Source *s):
  source(s)
{
  _max = -MAXVALUE;
  _min = MAXVALUE;
  _numberOfValues = 0;
  _isIncreasing = true;
  _isDecreasing = true;
  name = NULL;
  label = NULL;
  fieldReader = NULL;

  {
    char s[32];
    sprintf(s, "field %d", fieldCount++);
    setName(s);
  }
  
  // add this to the list of Fields in the source.
  source->push_back(this);
}

void Field::setName(const char *name_in)
{
  if(name_in && name_in[0])
  {
    if(name)
      free(name);

    name = strdup(name_in);
  
    if(!label)
      label = strdup(name_in);
  }
}

void Field::setLabel(const char *label_in)
{
  if(label_in && label_in[0])
  {
    if(label)
      free(label);
    
    label = strdup(label_in);
  }
}

Field::~Field(void)
{
  if(name)
    free(name);

  if(label)
    free(label);
  
  source->remove(this);
}


// Find the minimum values in the range of indexes indexing starts at
// 0.  Over-write this method if you've got a faster way in your
// Field.
value_t Field::max(count_t i_max, count_t i_min)
{
  rewind();
  count_t i;
  if(i_max == ((count_t) -1))
    i_max = _numberOfValues - 1;
  value_t x, Max = -MAXVALUE;
  if(i_min > 0)
    for(i=0;i<i_min;i++)
      read();
  for(i=i_min;i<=i_max;i++)
    if((x=read()) > Max)
      Max = x;
  rewind();
  return Max;
}

// Find the minimum values in the range of indexes indexing starts at
// 0. Over-write this method if you've got a faster way in your Field.
value_t Field::min(count_t i_max, count_t i_min)
{
  rewind();
  count_t i;
  if(i_max == ((count_t) -1))
    i_max = _numberOfValues - 1;
  value_t x, Min = MAXVALUE;
  if(i_min > 0)
    for(i=0;i<i_min;i++)
      read();
  for(i=i_min;i<=i_max;i++)
    if((x=read()) < Min)
      Min = x;
  rewind();
  return Min;
}
