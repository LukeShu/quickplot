/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */
#include "config.h"

#include <iostream>

#ifdef Darwin
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

#include "Source.h"
#include "Globel.h"



std::list<Source *> sources;


const char *Source::TYPE_STRING[] =
{
  "NONE",
  "SNDFILE",
  "ASCII_FILE"
};


Source::Source(void):
  fileName(NULL),
  baseFileName(NULL)
{
  // The inheriting class object will validate this or not.
  isValid = false;

  setType(NONE);

  // Add this to the list of all Source objects.
  sources.push_back(this);
};

void Source::addCloseMenus(const char *label)
{
  std::list<MainWindow *>::const_iterator win = app->begin();
  for(;win != app->end(); win++)
  {
    (new CloseSourceMenuItem(this, *win))->show();
  }
}

Source::~Source(void)
{
  closeSourceMenuItems.clear();
  if(isValid)
  {

    // delete all plots that use fields from this Source.

    if(app->size() > 0)
    {
      std::list<MainWindow *>::const_iterator win = app->begin();
      for(;win != app->end(); win++)
      {
        int i;
        int n = (*win)->graphsNotebook.get_n_pages();
        for(i=0;i<n;i++)
        {
          Graph *graph = dynamic_cast<Graph *>
            ((*win)->graphsNotebook.get_nth_page(i));
          if(graph)
          {
            
            std::list<Field *>::const_iterator field = begin();
            while(field != end())
            {
              
              bool got_plot = false;
              
              //opSpew << " file=" << __FILE__ << " line=" << __LINE__ << std::endl;
              std::list<Plot *>::const_iterator plot;
              for(plot=graph->begin(); plot != graph->end(); plot++)
              {
                
                if((*plot)->x() == (*field) || (*plot)->y() == (*field))
                {
                  delete (*plot);
                  got_plot = true;
                  // Each time we delete a Plot the Graph Plot list
                  // breaks and must be reinitialized so we can look
                  // for more plots with this Field.  It's because
                  // Plot::~Plot() uses the Graph list to remove
                  // itself.
                  break;
                }
              }
              // If we deleted no more Plots we go to the next Graph.
              if(!got_plot) field++;
              
            } // for(;field != end();)
            
          }
        }        
      }
    }
    
    // Remove and delete the fields in reverse order so that any
    // dependent fields can clean up.
    while(size() > 0) // We don't use an iterator here because the
                        // Field destructor removes itself from the
                        // list, and so invalidates the iterator if it
                        // was used.
    {
      delete (back()); // this will remove it fom the list.
    }
    
  }
 
  if(fileName)
    free(fileName);

  if(baseFileName)
    free(baseFileName);

  sources.remove(this);

  // If this was a valid loaded source let what-ever is lessoning
  // know that we are about to delete all the fields that are listed
  // in this.
  m_signal_removedSource.emit(this);



};


extern "C"
{  
  static gboolean deleteSLater(gpointer data)
  {
    gtk_idle_remove_by_data(data);
    Source *source = ((struct SourceDeleteLater *) data)->source;
    
    std::list<CloseSourceMenuItem *>::const_iterator it =
      source->closeSourceMenuItems.begin();
    for(;it != source->closeSourceMenuItems.end(); it++)
    {
      // remove the close file menu
      (*it)->mainWindow->menuBar.getFileMenu().items().remove(*(*it));
      // delete the close file menu
      delete (*it);
    }
    
    // delete the source.
    delete source;
    return ((gint) 0);
  }
}



CloseSourceMenuItem::CloseSourceMenuItem(Source *s, MainWindow *mainWindow_in):
  closeImage(Stock::CLOSE, ICON_SIZE_MENU),
  mainWindow(mainWindow_in)
{
  Glib::ustring str;
  str += "Close ";
  str += s->getFileName();
  add_label(str);
  
  set_image(closeImage);
  mainWindow->menuBar.getFileMenu().items().push_back(*this);
  signal_activate().
    connect(sigc::mem_fun(*this, &CloseSourceMenuItem::deleteSourceLater));
  closeImage.show();
  s->closeSourceMenuItems.push_back(this);
  dl.source = s;
}

void CloseSourceMenuItem::deleteSourceLater(void)
{
  gtk_idle_add(deleteSLater, &dl);
}


SigC::Signal1<void, Source *> Source::signal_addedSource()
{
  return m_signal_addedSource;
}

SigC::Signal1<void, Source *> Source::m_signal_addedSource;


SigC::Signal1<void, Source *> Source::signal_removedSource()
{
  return m_signal_removedSource;
}

SigC::Signal1<void, Source *> Source::m_signal_removedSource;

