/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */
 
#include <stdlib.h>
#include <string.h>

#include <gtkmm.h>

#include "value_t.h"
#include "Globel.h"
#include "FileList.h"


FileList *FileList::first = NULL;

FileList::FileList(char *filename)
{
  if(filename)
    fileName = strdup(filename);
  else
    fileName = NULL;

  skipLines = 0;
  readLabels = 0;
  labelSeparator = LABEL_SEPARATOR;
  hasLinearField = false;
  linearFieldStep  = (value_t) 1.0;
  linearFieldStart = (value_t) 0.0;
  takeLog = false;
  
  next = NULL;

  if(first)
    {
      FileList *l = first;
      while(l->next)
	l = l->next;
      l->next = this;
    }
  else
    first = this;
}

void FileList::setFileName(const char *filename)
{
  if(fileName)
    free(fileName);

  if(filename)
    fileName = strdup(filename);
  else
    fileName = NULL;
}

FileList::~FileList(void)
{
  if(fileName)
    {
      free(fileName);
      fileName = NULL;
    }
  FileList *l = first, *prev = NULL;
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
