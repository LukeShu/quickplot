/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */
 
#include <stdlib.h>

#include "PlotList.h"


PlotList *PlotList::first = NULL;

PlotList::PlotList(void)
{
  next = NULL;

  if(first)
    {
      PlotList *l = first;
      while(l->next)
	l = l->next;
      l->next = this;
    }
  else
    first = this;

  
  x = y = NULL;
  numPlots = 0;
  index = 0;
}

void PlotList::add(int X, int Y)
{
  numPlots++;
  x = (int *) realloc(x, sizeof(int)*numPlots);
  y = (int *) realloc(y, sizeof(int)*numPlots);

  x[numPlots-1] = X;
  y[numPlots-1] = Y;
}

void PlotList::rewind(void)
{
  index = 0;
}

void PlotList::get(int &X, int &Y)
{
  X = x[index];
  Y = y[index];
  index++;
}

PlotList::~PlotList(void)
{
  if(x)
    free(x);
  if(y)
    free(y);
  
  PlotList *l = first, *prev = NULL;
  for(;l;l = l->next)
    {
      if(this == l)
	break;
      prev = l;
    }
  if(l) // this == l
    {
      if(prev)
	prev->next = next;
      else // this == first
	first = next;
    }
}
