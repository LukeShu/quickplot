/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */
#include "config.h"

#include <values.h>
#include <list>
#include <iomanip>
//#include <algorithm>

#include <gtkmm.h>


using namespace Gtk;
#include "errorStr.h"
#include "value_t.h"
#include "Globel.h"
#include "Source.h"
#include "FileList.h"
#include "File.h"
#include "Field.h"
#include "ListField.h"
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


#include "errorStr.h"
#include "FileReader.h"

#define IS_SEPARATOR_CHAR(x)  (!(((x)>='0'&&(x)<='9')||(x)=='\n'||(x)=='.'|| \
                            (x)=='-'||(x)=='+'||(x)=='e'||(x)=='E'||(x)<01))

// The maximum number of chars that can be in a number when it is read
// in in a ASCII file.
#define NUM_STR_SIZE   ((size_t) 128)

// return values from File::getValue()
#define READERROR  001
#define ENDOFFILE  002
#define ENDOFLINE  004
#define GOTVALUE   010

#define LINE_END   ('\n')

// lastNum is used in File::getValue(FileReader *file, value_t *val)
static char lastNum = '\0';

static count_t numberOfLines=0;


// This will get a value as a value_t.
int File::getValue(FileReader *file, value_t *val)
{
  int i= file->readChar();
  if(!lastNum)
    while((IS_SEPARATOR_CHAR((char) i) || ((char) i) == LINE_END) && i!=EOF)
      {
	i = file->readChar();
	if(((char) i) == LINE_END)
	  numberOfLines++;
      }

  size_t j = 0;
  char buf[NUM_STR_SIZE+1];
  // Put this last char that was read in the last time this was
  // called.
  if(lastNum)
    {
      buf[j++] = lastNum;
      lastNum = '\0';
    }
  while(!IS_SEPARATOR_CHAR((char) i) && ((char) i)!=LINE_END &&
	i!=EOF && j<NUM_STR_SIZE)
    {
      buf[j++] = i;
      i = file->readChar();
    }
  if(error)
    return READERROR;
  else if(j==NUM_STR_SIZE)
    {
      if(!opSilent)
        opSpew << "quickplot ERROR: bad data in file '" << fileName
               << "' on line number " << numberOfLines+1 << std::endl;
      error = 1;
      return READERROR;
    }
  if(j==0 && i==EOF)
    return ENDOFFILE;

  buf[j] = '\0';
  
  //opSpew << std::endl << __LINE__ << " file=" << __FILE__
  //          << " buf=" << buf
  //          <<std::endl;

  int returnVal = GOTVALUE;
  if(i!=EOF)
    {
      while((IS_SEPARATOR_CHAR((char) i) || ((char) i) == LINE_END) && i!=EOF)
	{
	  if(((char) i) == LINE_END)
	    {
	      numberOfLines++;
	      returnVal |= ENDOFLINE;
	    }
	  i = file->readChar();
	}
      if(i!=EOF)
	lastNum = i;
    }

  if(i == EOF)
    returnVal |= ENDOFFILE;

  errno = 0;
  char *ptr = buf;
  *val = STRINGTOVALUE(buf, &ptr);
  if(ptr==buf || errno)
    {
      if(!opSilent)
        opSpew << "quickplot ERROR: bad data in file '" << fileName
               << "' on line number " << numberOfLines+1 << std::endl;
      error = 1;
      lastNum = '\0';
      return READERROR;
    }
  
  return returnVal;
}


bool File::readASCIIFile(FILE *file_in, const FileList *fileList)
{
  FileReader file(fileName, file_in);
  if(error)
  {
    return false;
  }
  
  numberOfLines=0;
  lastNum = '\0';
  
  int i=0;
  // Skip fileList->skipLines lines in the file.
  for(;i!=EOF && numberOfLines < fileList->skipLines; numberOfLines++)
    for(i=file.readChar(); i!=EOF && LINE_END != (char) i;)
      i = file.readChar();

  // Skip labels from the file for now.
  if(i!=EOF && fileList->readLabels)
    for(i=0;LINE_END != (char) i && i!=EOF;)
      i = file.readChar();

  if(i==EOF)
    {
      if(!opSilent)
        opSpew << "quickplot ERROR: reading file '" << fileName
               << "': file is too short." << std::endl;
      error = 1;
      return false;
    }

  // Count the number of Fields.
  value_t val;
  // Reset File::getValue(FileReader *file, value_t *val)
  lastNum = '\0'; 

  int numberOfFields = 0;
  
  for(i=0;!(i&ENDOFFILE) && !(i&READERROR) && !(i&ENDOFLINE);numberOfFields++)
    i = getValue(&file, &val);

  if(numberOfFields < 1)
    {
      if(fileList->skipLines < 1)
      {
	if(!opSilent)
            opSpew << "quickplot ERROR: reading file '" << fileName
                 << "': contains no fields." << std::endl;
        
      }
      else
      {
        if(!opSilent)
          opSpew << "quickplot ERROR reading file '" << fileName
                 << "': read no fields after skipping the first "
                 << fileList->skipLines << " lines." << std::endl;
      }
      error = 1;
      return false;
    }
#if 0 // Including this block will disable the possibility of loading
      // fields with one value.
  else if(i&ENDOFFILE)
    {
      if(!opSilent)
        opSpew << "quickplot ERROR: reading file '" << fileName
               << "': file is too short, and can "
               << "only load one value per field." << std::endl;
      error = 1;
      return false;
    }
#endif
  else if(i&READERROR)
    return false; // error
  

  // add all the new fields for the data to read into.
  ListField<value_t> *firstField = new ListField<value_t>(this);
  {
    ListField<value_t> *f = firstField;
    for(i=0;i<numberOfFields-1;i++)
    {
      f->next = new ListField<value_t>(this);
      f = f->next;
    }
    f->next = NULL;
  }

  file.rewind();
  file.freeRewind();
  numberOfLines=0;

  // Skip fileList->skipLines lines in the file.
  //printf("skipping %d\n",fileList->skipLines);
  for(i=0;i!=EOF && numberOfLines < fileList->skipLines; numberOfLines++)
    {
      //printf("%d\n",numberOfLines);
      for(i=file.readChar(); i!=EOF && LINE_END!=(char) i;)
	i = file.readChar();
    }

  // Read labels from the file.
  if(i!=EOF && fileList->readLabels)
    {
      int fieldCount=0;
      i = 0;
      
      ListField<value_t> *field = firstField;
      
      while(LINE_END != ((char) i) &&i!=EOF && field)
      {
        i = file.readChar();
        while(fileList->labelSeparator == (char) i)
          i = file.readChar();
        
#define NAME_LENGTH 32
        char label[NAME_LENGTH+1];
        int j=0;
        if(i!=EOF && LINE_END != (char) i)
          label[j++] = i;
        while(i!=EOF &&
              LINE_END != (char) i &&
              fileList->labelSeparator != (char) i &&
              j<NAME_LENGTH)
          label[j++] = i = file.readChar();
        label[j-1] = '\0';
        if(j==NAME_LENGTH)
          while(i!=EOF &&
                LINE_END != (char) i &&
                fileList->labelSeparator != (char) i)
            i = file.readChar();
        
        if(j>0)
          field->setLabel(label);

        field = field->next;
        
        if(!field) break;
        fieldCount++;
      }
      numberOfLines++;
	
      // If there were not enough labels in the line make some.
      for(;field;field = field->next)
      {
        char s[NAME_LENGTH];
        snprintf(s,NAME_LENGTH,"%s", baseFileName);
        field->setLabel(s);
        snprintf(s,NAME_LENGTH,"%d", fieldCount++);
        field->setName(s);
      }
      
      // Read to the end of the line.
      while(i!=EOF &&
            LINE_END != (char) i)
        i = file.readChar();
    }
  else
  {
    // make some default field labels.
    ListField<value_t> *field = firstField;
    int fieldCount=0;
    for(;field;field = field->next)
    {
      char s[NAME_LENGTH];
#if 0
      if(fileList->takeLog)
        snprintf(s,NAME_LENGTH,"Log[%s]", baseFileName);
      else
#endif
        snprintf(s,NAME_LENGTH,"%s", baseFileName);
      field->setLabel(s);

      
      snprintf(s,NAME_LENGTH,"%d", fieldCount++);
      field->setName(s);
    }
  }
  

  if(i==EOF)
    {
      if(!opSilent)
        opSpew << "quickplot ERROR: reading file '" << fileName
               << "': file is too short." << std::endl;
      error = 1;
      return false;
    }

  // Reset File::getValue(FileReader *file, value_t *val)
  lastNum = '\0';
  i = 0;

  // read data into the fields
  while(!(i&ENDOFFILE) && !(i&READERROR))
    {
      ListField<value_t> *field = firstField;
      int j=0;
      for(;j<numberOfFields-1;j++)
	{
	  i = getValue(&file, &val);
	  if(i!=GOTVALUE)
	    {

              if(!opSilent)
                opSpew << "quickplot ERROR: reading file '" << fileName
                       << "': near line number " << numberOfLines+1
                       << std::endl;
              error = 1;
              setType(ASCII_FILE);
              return true;
	    }
#if 0
          if(fileList->takeLog)
          {
            if(val <= (value_t) 0.0)
            {
              if(!opSilent)
                opSpew << "quickplot ERROR: reading file '" << fileName
                       << "': near line number " << numberOfLines+1
                       << ": can't compute the log of value="
                       << val << std::endl;
              error = 1;
              setType(ASCII_FILE);
              return true;
            }
            field->write(LOG10(val));
          }
          else
#endif
            field->write(val);
	  field = field->next;
	}
      i = getValue(&file, &val);
      if(!(i&GOTVALUE && (i&ENDOFFILE || i&ENDOFLINE)))
	{
          if(!opSilent)
            opSpew << "quickplot ERROR: reading file '" << fileName
                   << "': near line number " << numberOfLines+1
                   << std::endl;
          error = 1;
          setType(ASCII_FILE);
          return true;
	}
#if 0
      if(fileList->takeLog)
      {
        if(val <= (value_t) 0.0)
        {
          if(!opSilent)
            opSpew << "quickplot ERROR: reading file '" << fileName
                   << "': near line number " << numberOfLines+1
                   << ": can't compute the log of value="
                   << val << std::endl;
          error = 1;
          setType(ASCII_FILE);
          return true;
        }
        field->write(LOG10(val));
      }
      else
#endif
        field->write(val);
    }

  if(i&READERROR)
    {
      error = 1;
      setType(ASCII_FILE);
      return true;
    }

  if((*begin())->numberOfValues() < 1)
    {
      if(fileList->skipLines < 1)
      {

         if(!opSilent)
            opSpew << "quickplot ERROR: reading file '" << fileName
                   << "': read no fields." << std::endl;
      }
      else
      {
	if(!opSilent)
            opSpew << "quickplot ERROR: reading file '" << fileName
                   << "': read no fields after skipping the first "
                   << fileList->skipLines << "lines." << std::endl;
      }
      error = 1;
      setType(ASCII_FILE);
      return true;
    }

  // For adding a Linear sequence field if requested or needed.  We
  // need to add it before we create the other fields so that it is
  // the first in the list of fields in this File object.  We add this
  // Linear sequence field so that the File object has two Fields at
  // least.
  bool hasLinearField = fileList->hasLinearField;
  
  if(hasLinearField || numberOfFields == 1)
  {
    LinearField *lf =
      new LinearField(this, fileList->linearFieldStep, fileList->linearFieldStart);
    // make it first in the list
    remove(lf);
    push_front(lf);
    numberOfFields++;
    hasLinearField = true;
  }

  if(opVerbose)
  {
    // C++ streams suck!  It's twice the code of C streams and it runs
    // slower.
    opSpew << "Read ASCII file " << fileName << std::endl
           << "created: "
           << ((hasLinearField)? " 1 linear field and ":"")
           << numberOfFields-((hasLinearField)?1:0)<< " ListFields with "
           << firstField->numberOfValues() << " values in each." << std::endl;
  }
  
  setType(ASCII_FILE);
  return true; // success
}

