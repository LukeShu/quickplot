/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>

#include <gtkmm.h>

#include "value_t.h"
#include "FileReader.h"
#include "errorStr.h"
#include "Globel.h"

FileReader::FileReader(const char *filename_in, FILE *file_in)
{
  isFree = 0;
  openedFile = 0;
  file = NULL;
  buffer = NULL;
  bufferSize = 0;
  in = 0;
  out = 0;
  error = 0;
  // user end of file
  endOfFile = 0;
  // if end of file is set
  _endOfFile = 0;

  if(!file_in)
    {
      file = fopen(filename_in, "r");
      if(!file)
	{
	  snprintf(errorStr, ERRORSTR_LENGTH,
		   "quickplot ERROR: Failed to open file '%s' for reading: "
		   "system error number %d: %s.",
		   filename_in, errno, strerror(errno));
          if(!opSilent)
            opSpew << errorStr << std::endl;
	  error = 1;
	  endOfFile = 1;
	  return;
	}
      openedFile = 1;
    }
  else
    file = file_in;

}

FileReader::~FileReader(void)
{
  if(buffer)
    {
      free(buffer);
      buffer = NULL;
      bufferSize = 0;
    }
  if(file && openedFile)
    {
      fclose(file);
      file = NULL;
      openedFile = 0;
    }
}


void FileReader::rewind(void)
{
  out = 0;
  endOfFile = 0;
}

void FileReader::freeRewind(void)
{
  isFree = 1;
}

int FileReader::readChar(void)
{
  if(isFree && in == out)
    {
      if(_endOfFile) return EOF;
      
      // no longer buffered.
      int i = fgetc(file);
      if(i == EOF)
	_endOfFile = endOfFile = 1;
      //printf("%c", (char) i);
      return i;
    }
  // enough data in the buffer.
  else if(in > out)
    {
      //printf("%c", (char) buffer[out]);
      return ((int) buffer[out++]);
    }
  else if(_endOfFile)
  {
    return EOF;
  }
  else // If need more data in the buffer.
    {
      // is the buffer big enough
      if(in + 1 > bufferSize)
        buffer = (unsigned char *)
          realloc(buffer, sizeof(unsigned char)*(bufferSize += CHUNK));
      
      int i = fgetc(file);
        
      if(i == EOF)
      {
	_endOfFile = endOfFile = 1;
        return EOF;
      }
      else
      {
        buffer[in++] = ((unsigned char) i);
        out = in;
        //printf("%c", (char) buffer[in-1]);
        return i;
      }
    }
}
