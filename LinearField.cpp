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
#include "LinearField.h"
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

void *LinearField::makeDequeuer(count_t number_of_values)
{
  struct Dequeuer *d = (struct Dequeuer *) malloc(sizeof(struct Dequeuer));
  d->numberOfValuesMinus1 =
    ((number_of_values<_numberOfValues)?number_of_values:_numberOfValues)
    -1;
  d->index = 0;
  return (void *) d;
}

void LinearField::destroyDequeuer(void *dequeuer)
{
  free(dequeuer);
}

