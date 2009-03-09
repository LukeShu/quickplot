/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */


// Globel option flags effect can be changed after start up.

#define LABEL_SEPARATOR_STR   " "
#define LABEL_SEPARATOR       LABEL_SEPARATOR_STR[0]

// In sizes in pixels
#define MAX_GRID_LINE_WIDTH    40
#define MAX_PLOT_LINE_WIDTH    40
#define MAXMAX_PLOT_LINE_WIDTH 200
#define MAX_POINT_SIZE         40
#define MAXMAX_POINT_SIZE      200

#define MAX_GRIDXLINESPACE   1500
#define MIN_GRIDXLINESPACE   1
#define MAX_GRIDYLINESPACE   1500
#define MIN_GRIDYLINESPACE   1



// Are todays C++ compilers smart enough to put many (32 or 64) bool
// types in one unsigned integer?


extern bool opVerbose; // more spew then default
extern bool opSilent;  // no spew, even on error
extern std::ostream &opSpew;

extern guint opPickButton;
extern guint opZoomButton;
extern guint opMidButton;





// at startup
extern bool opShowMenuBar; 
extern bool opShowButtons; 
extern bool opShowGraphConfig;
extern bool opShowGraphTabs;
extern bool opShowStatusBar;

// start up and when loading a file from GUI
extern bool opNoDefaultPlots;
extern int opMaxNumDefaultPlots; // plots per Graph



// main window geometry
extern char *opGeometry;


// class Graph default properties.  Since there can be more then one
// Graph (Tab) and each Graph can change their properties, these are
// just startup defaults.

extern bool opSameScale;
extern bool opAutoSameScale;
extern bool opShowAutoGrid;
extern int  opGridLineWidth;
extern int  opGridXLineSpace;
extern int  opGridYLineSpace;


// class Plot default properties.  Since there can be more then one
// Plot and each Plot can change their properties, these are just
// startup defaults.

extern bool opShowLines;
extern bool opShowPoints;

// How these defaults are interpeted depends on the point and line type.
extern int opPointSize; // pixels. A diameter like measure.
extern int opLineWidth; // pixels. A diameter (width) like measure.

// This is set to show that the user has used a command line option to
// set opShowLines, opShowPoints, opPointSize or opLineWidth from the
// command line.
extern bool opUserSetLineOrPointOption;


// default LogField option
extern value_t opLogBase;
