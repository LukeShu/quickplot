/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */

/* class ColorGen generates all the default colors for Quickplot
 * Graphs */

#include <glib.h>
#include "ColorGen.h"

// color= default line and point colors
const gushort ColorGen::color[NUM_COLORS][3] =
{
  { 228*256, 45*256, 4*256 },
  { 101*256, 255*256, 54*256 },
  { 224*256, 18*256, 224*256 },
    
  { 0, 189*256, 203*256 },
  { 40*256, 17*256, 211*256 },
  { 225*256, 229*256, 12*256 },
    
  { 45*256, 148*256, 32*256},
  { 148*256, 61*256, 32*256},
  { 33*256, 154*256, 154*256},
    
  { 122*256, 31*256, 142*256},
  { 157*256, 149*256, 34*256},
  { 31*256, 50*256, 143*256}
};

// default grid color
gushort ColorGen::gridColor[3] =
{
  255*256/3, 255*256/3, 255*256/3
};

// default graph background color
gushort ColorGen::backgroundColor[3] =
{
  0, 0, 0
};


ColorGen::ColorGen(void)
{
  count = 0;
}

void ColorGen::getGridColor(gushort &r, gushort &g, gushort &b)
{
  r = gridColor[0];
  g = gridColor[1];
  b = gridColor[2];
}

void ColorGen::getBackgroundColor(gushort &r, gushort &g, gushort &b)
{
  r = backgroundColor[0];
  g = backgroundColor[1];
  b = backgroundColor[2];
}


// get line and point colors.
void ColorGen::getColors(gushort &lr, gushort &lg, gushort &lb,
                         gushort &pr, gushort &pg, gushort &pb)
{
  lr = color[count%NUM_COLORS][0];
  lg = color[count%NUM_COLORS][1];
  lb = color[count%NUM_COLORS][2];
  
  pr = color[(count+NUM_COLORS/2)%NUM_COLORS][0];
  pg = color[(count+NUM_COLORS/2)%NUM_COLORS][1];
  pb = color[(count+NUM_COLORS/2)%NUM_COLORS][2];

  count++;
  
  if(count >= NUM_COLORS*32)
    count = 0;
}
