/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */
#include "config.h"

#ifdef USE_LIBSNDFILE

#include <iostream>

#ifdef QP_ARCH_DARWIN
# include <limits.h>
# include <float.h>
#else
# include <values.h>
#endif

#include <list>
#include <iomanip>
#include <sndfile.h>

#include <gtkmm.h>

using namespace Gtk;
#include "errorStr.h"
#include "value_t.h"
#include "Field.h"
#include "LinearField.h"
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


/* This opens and reads in a sound file using libsndfile.
 */
bool File::readSndFile(const FileList *fileList, int fd)
{
  using std::endl;
  SF_INFO info;
  ::SNDFILE * sndfile = NULL;

  if(fd == -1)
    sndfile = sf_open(getFileName(), SFM_READ, &info);
  else
    sndfile = sf_open_fd(fd, SFM_READ, &info, 1 /* close fd */);

  if(!sndfile || info.frames < (sf_count_t) 1)
  {
    if(opVerbose)
      opSpew << "quickplot INFO: Can't read file " << fileName << endl
             << "with libsndfile as a sound file." << endl;
    return false; // not a sound file.
  }
  
  if(opVerbose)
  {
    // C++ streams suck!  It's twice the code of C streams and it runs
    // slower.
    opSpew << "quickplot Reading sound file " << fileName << endl
           << "----------------------------------" << endl
           << "frames     = " << info.frames << endl
           << "samplerate = " << info.samplerate << endl
           << "channels   = " << info.channels << endl
           << "format     = 0x" << std::setw(8) <<
      std::setfill('0') << std::hex << info.format <<
      std::setw(0) << std::dec << endl
           << "seekable   = " << info.sections << endl
           << "fmt_check  = " << sf_format_check(&info) << endl;
  }
  

  int soundSubType = SF_FORMAT_SUBMASK & info.format;
  bool returnVal = true;

  // handle all libsndfile Subtypes
  switch (soundSubType)
  {
    case SF_FORMAT_PCM_S8:
    case SF_FORMAT_PCM_16:
    case SF_FORMAT_PCM_U8:
    case SF_FORMAT_DWVW_12:
    case SF_FORMAT_DWVW_16:
      readSndShort(sndfile, info.samplerate,
                   static_cast<count_t>(info.frames),
                   info.channels);
      break;
    
    case SF_FORMAT_PCM_24:
    case SF_FORMAT_PCM_32:
    case SF_FORMAT_ULAW:
    case SF_FORMAT_ALAW:
    case SF_FORMAT_IMA_ADPCM:
    case SF_FORMAT_MS_ADPCM:
    case SF_FORMAT_GSM610:
    case SF_FORMAT_VOX_ADPCM:
    case SF_FORMAT_G721_32:
    case SF_FORMAT_G723_24:
    case SF_FORMAT_G723_40:
    case SF_FORMAT_DWVW_24:
    case SF_FORMAT_DWVW_N:
      readSndInt(sndfile, info.samplerate,
                 static_cast<count_t>(info.frames),
                 info.channels);
      break;

    case SF_FORMAT_FLOAT:
      readSndFloat(sndfile, info.samplerate,
                   static_cast<count_t>(info.frames),
                   info.channels);
      break;

    case SF_FORMAT_DOUBLE:
      readSndDouble(sndfile, info.samplerate,
                   static_cast<count_t>(info.frames),
                   info.channels);
      break;
  }
     
  
  sf_close(sndfile);
  setType(Source::SNDFILE);


  if(!error && fileList->hasLinearField)
  {
    // For adding a Linear sequence field if requested.  We need to
    // add it before we create the other fields so that it is the
    // first in the list of fields in this File object.
    LinearField *lf =
      new LinearField(this, fileList->linearFieldStep, fileList->linearFieldStart);
    // make it first in the list
    remove(lf);
    push_front(lf);
  }
  
  return returnVal;
}

#endif // #ifdef USE_LIBSNDFILE
