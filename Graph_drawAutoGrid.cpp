/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */
#include "config.h"

#include <values.h>
#include <stdio.h>
#include <math.h>

#include <gtkmm.h>
#include <pangomm/layout.h>


using namespace Gtk;
#include "value_t.h"
#include "ValueSlider.h"
#include "PlotLister.h"
#include "PlotConfig.h"
#include "MainMenuBar.h"
#include "ButtonBar.h"
#include "StatusBar.h"
#include "MainWindow.h"
#include "App.h"

#include "Field.h"
#include "Plot.h"
#include "ColorGen.h"
#include "Graph.h"
#include "Globel.h"



#define ABSVAL(x)  (((x) > ((value_t) 0.0)) ? (x) : (-(x)))
#define TICKMIN  5.0
#define TICKMAX  8.0

#define SMALL_NUM   ABSVAL(MINVALUE)
#define ZERO        ((value_t) 0.0)
#define ONE         ((value_t) 1.0)
#define TEN         ((value_t) 10.0)


// Auto draw a x-y grid.  The code in this method is not too pretty.
// This will draw a x-y grid with value labels on each line.  This
// grid keeps somewhat even spacing when the Graph window resizes.
// The x-y values of the grid lines try to stay "nice".

void Graph::drawAutoGrid(Glib::RefPtr<Gdk::Drawable> _win,
                         Glib::RefPtr<Gdk::GC> _gc)
{
  // check that the plot scales are consistant with this Graph.
  (*begin())->preDrawCheckResize();
  
  value_t scaleX = (*begin())->currentZoomLevel->scaleX;
  value_t shiftX = (*begin())->currentZoomLevel->shiftX;
  value_t scaleY = (*begin())->currentZoomLevel->scaleY;
  value_t shiftY = (*begin())->currentZoomLevel->shiftY;

  // Find the x,y max and min values in the window.
  value_t xmax = (get_width() - shiftX)/scaleX;
  value_t xmin = (0 - shiftX)/scaleX;
  value_t ymax = (0 - shiftY)/scaleY;
  value_t ymin = (get_height() - shiftY)/scaleY;


  /***************************************************/
  /*        get the x value lines calculated         */
  /***************************************************/

  //printf("get_width()=%d xmin=%g xmax=%g\n", get_width(), xmin, xmax);

  value_t delta = (xmax - xmin)/(2 + ((value_t) get_width()/gridXLineSpace));

  //printf("xdelta=%g\n", delta);

  int delta_p = (int) log10(ABSVAL(delta) > SMALL_NUM ? ABSVAL(delta) : SMALL_NUM);

  if(delta_p <= 0) delta_p--;

  // printf("xdelta_p=%d\n", delta_p);

  value_t xpow_part = POW(TEN, delta_p);
  {
    // strip off the round-off error off of xpow_part.
    char s[16];
    //for example: xpow_part=0.20005465 --> s=0.20 --> ypow_part=0.2000000
    sprintf(s, TWO_DIGIT_FORMAT, xpow_part);
    sscanf(s, SCAN_FORMAT, &xpow_part);
    //printf("s=%s  xpow_part=%.15g\n",s, xpow_part);
  }
  
  int64_t xmin_mat = (int64_t) (xmin/xpow_part);
  int64_t xmax_mat = (int64_t) (xmax/xpow_part);

  //printf(" xmin = %lldx10%+d xmax = %lldx10%+d\n",
  // xmin_mat, delta_p, xmax_mat, delta_p);

  int64_t xinc;

  if(get_width() > gridXLineSpace)
    xinc = (int64_t) ((xmax_mat-xmin_mat)/((value_t) get_width()/gridXLineSpace));
  else
    xinc = xmax_mat-xmin_mat;

  //printf("               xinc=%lld\n", xinc);

  // round to 1, 2, 5, times 10^N
  if     (xinc < (int64_t) 2)   xinc = 1;
  else if(xinc < (int64_t) 5)   xinc = 2;
  else if(xinc < (int64_t) 10)  xinc = 5;
  else if(xinc < (int64_t) 20)  xinc = 10;
  else if(xinc < (int64_t) 50)  xinc = 20;
  else if(xinc < (int64_t) 100) xinc = 50;
  else if(xinc < (int64_t) 200) xinc = 100;
  else                          xinc = 200;

  int64_t xstart = xmin_mat;

  //printf("xinc=%lld\n", xinc);

  if(xmin_mat > (int64_t) 0 && xmin_mat%xinc)
    xstart -= xmin_mat%xinc;
  else if(xmin_mat < (int64_t) 0 && xmin_mat%xinc)
    xstart -= xinc + xmin_mat%xinc;

  
  /***************************************************/
  /*        get the y value lines calculated         */
  /***************************************************/

  delta = (ymax - ymin)/(2 + ((value_t) get_height()/gridYLineSpace));
 
  delta_p = (int) log10(ABSVAL(delta) > SMALL_NUM ? ABSVAL(delta) : SMALL_NUM);

  if(delta_p <= 0) delta_p--;

  //printf("ydelta_p=%d\n", delta_p);

  value_t ypow_part = POW(TEN, delta_p);
  {
    // strip off the round-off error off of ypow_part.
    char s[16];
    //for example: ypow_part=0.20005465 --> s=0.20 --> ypow_part=0.2000000
    sprintf(s, "%.2g", ypow_part);
    sscanf(s, "%lf", &ypow_part);
  }

  int64_t ymin_mat = (int64_t) (ymin/ypow_part);
  int64_t ymax_mat = (int64_t) (ymax/ypow_part);

  //printf(" ymin = %lldx10%+d ymax = %lldx10%+d\n",
  // ymin_mat, delta_p, max_mat, delta_p);

  int64_t yinc;

  if(get_height() > gridYLineSpace)
    yinc = (int64_t) ((ymax_mat-ymin_mat)/((value_t) get_height()/gridYLineSpace));
  else
    yinc = ymax_mat-ymin_mat;

  if     (yinc < (int64_t) 2)   yinc = 1;
  else if(yinc < (int64_t) 5)   yinc = 2;
  else if(yinc < (int64_t) 10)  yinc = 5;
  else if(yinc < (int64_t) 20)  yinc = 10;
  else if(yinc < (int64_t) 50)  yinc = 20;
  else if(yinc < (int64_t) 100) yinc = 50;
  else if(yinc < (int64_t) 200) yinc = 100;
  else                          yinc = 200;


  int64_t ystart = ymin_mat;

  //printf("yinc=%lld\n", yinc);

  if(ymin_mat > (int64_t) 0 && ymin_mat%yinc)
    ystart -= ymin_mat%yinc;
  else if(ymin_mat < (int64_t) 0 && ymin_mat%yinc)
    ystart -= yinc + ymin_mat%yinc;


  /******************************************************/
  /* find where to put the value text for x and y lines */
  /******************************************************/

  int x_ylinetext = (int) ((xstart + xinc*0.2)*xpow_part*scaleX + shiftX);
  if(x_ylinetext < 3)
    x_ylinetext = (int) ((xstart + xinc*1.2)*xpow_part*scaleX + shiftX);

  int y_xlinetext = (int) ((ystart + yinc*0.5)*ypow_part*scaleY + shiftY);
  if(y_xlinetext > get_height() - 20)
    y_xlinetext = (int) ((ystart + yinc*1.5)*ypow_part*scaleY + shiftY);

  

  /****************************************************/
  /*              setup for drawing                   */
  /****************************************************/
  
  _gc->set_foreground(gridColor);
  _gc->set_line_attributes(gridLineWidth, Gdk::LINE_SOLID,
                           Gdk::CAP_ROUND, Gdk::JOIN_ROUND);
  if(pangolayout.is_null() && showGridNumbers)
  {
    pangolayout = create_pango_layout("");
  }

  
  /***************************************************/
  /*             draw the y lines                    */
  /***************************************************/

  int64_t i;

  
  for(i=ystart;i<=ymax_mat;i += yinc)
    {
      char str[64];
      snprintf(str,64, GRID_PRINT_FORMAT, i*ypow_part);
      //snprintf(str,64, "%*.*g", digit_count,digit_count, i*ypow_part);
      //printf(GRID_PRINT_FORMAT" ", str);
      int y = (int) (i*ypow_part*scaleY + shiftY);
      if(showGridNumbers)
      {
        pangolayout->set_text((const char *) str);
        _win->draw_layout(_gc, x_ylinetext, y+gridLineWidth/2, pangolayout);
      }
      
      _win->draw_line(_gc, 0, y, get_width(), y);
    }
  //printf("\n");


  /***************************************************/
  /*             draw the x lines                    */
  /***************************************************/

  for(i=xstart;i<=xmax_mat;i += xinc)
    {
      char str[64];
      snprintf(str,64, GRID_PRINT_FORMAT, i*xpow_part);

      //printf(" xpow_part=%.15g\n",xpow_part);
      
      //printf(GRID_PRINT_FORMAT" ", i*xpow_part);
      int x = (int) (i*xpow_part*scaleX + shiftX);
      
      if(showGridNumbers)
      {
        pangolayout->set_text((const char *) str);
        _win->draw_layout(_gc, x+6+gridLineWidth/2, y_xlinetext, pangolayout);
      }
      
      _win->draw_line(_gc, x, 0, x, get_height());
    }
  //printf("\n");
}
