/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */

#include <values.h>
#include <iostream>
#include <gdkmm/color.h>
#include <gdkmm/colormap.h>
#include <gdkmm/visual.h>

#include "value_t.h"
#include "Globel.h"


bool opVerbose = false; // more spew then default
bool opSilent = false;  // no spew, even on error
std::ostream &opSpew = std::cout;



// tipical values are mouse button 1, 2, or 3.
guint opPickButton = 1;
guint opZoomButton = 3;


// just at startup
bool opShowMenuBar = true;
bool opShowButtons = true;
bool opShowGraphConfig = false;
bool opShowGraphTabs = true;
bool opShowStatusBar = true;


// start up and when loading a file from GUI
bool opNoDefaultPlots = false;
int opMaxNumDefaultPlots = 12; // plots per Graph

// main window geometry
char *opGeometry = NULL;


// class Graph default properties.  Since there can be more then one
// Graph (Tab) and each Graph can change their properties, these are
// just startup defaults.

bool opSameScale     = false; // strictly the scales are the same on
                              // all graph plots. opSameScale
                              // over-rides opAutoScale.

bool opAutoSameScale = true;  // The scales are the same if it works
                              // well.
bool opShowAutoGrid  = true;
int  opGridLineWidth = 3;
int  opGridXLineSpace =  290;
int  opGridYLineSpace =  170;
//int  opGridXLineSpace =  390;
//int  opGridYLineSpace =  270;



// class Plot default properties.  Since there can be more then one Plot and
// each Plot can change their properties, these are just startup
// defaults.

bool opShowLines   = true;
bool opShowPoints  = true;

// How these defaults are interpeted depends on the point and line type.
int  opPointSize = 5; // pixels. A diameter like measure.
int  opLineWidth  = 3; // pixels. A diameter (width) like measure.

// This is set to show that the user has used a command line option to
// set opShowLines, opShowPoints, opPointSize or opLineWidth from the
// command line.
bool opUserSetLineOrPointOption = false;


// default LogField option
value_t opLogBase = (value_t) 10;
