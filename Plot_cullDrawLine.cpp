/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */
#include "config.h"

#ifdef Darwin
# include <limits.h>
# include <float.h>
#else
# include <values.h>
#endif

#include <gtkmm.h>


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


#define ABSVAL(x)  (((x) > (value_t)0.0) ? (x) : (-(x)))
#define ZRO ((value_t) 0.0)
#define ONE ((value_t) 1.0)

#define LG  (MAXVALUE/8)


// The old quickplot, from 2003, used a line and circle to cull.  This
// uses a brute force linear method with the most likely cases first.
// This is lots more code than the old quickplot but this may run much
// faster since there are no square roots to calculate.  The big
// pay-off will be with phase plots with lots of bunched data.


// Cull and Draw.  We hope that then we draw to point just a little
// (~10 pixels) outside of the window there is no problem.  Culling is
// required because zooming may generate numbers to plot like 1.0e+10
// which when converted to an int will give unknown results and draw
// lines all over the place.  Uncomment the //#define WITHOUT_CULLING
// to see what happens when you don't cull.  Culling also increases
// speed.  Not drawing is much faster than drawing.

// Just for testing.
//#define WITHOUT_CULLING

void Plot::cullDrawLine(value_t fromX, value_t fromY,
                        value_t toX, value_t toY)
{
#ifdef WITHOUT_CULLING
  // Try it with out culling.
  (this->*drawLineFunc)((int) fromX, (int) fromY, (int) toX, (int) toY);
  return;
#else

  
  // The common cases come first.
  
  // If both points are in the window.
  if(fromX > -lineWidthPlus1 && fromX < widthPlus &&
     toX > -lineWidthPlus1 && toX < widthPlus &&
     fromY > -lineWidthPlus1 && fromY < heightPlus &&
     toY > -lineWidthPlus1 && toY < heightPlus)
  {
    (this->*drawLineFunc)((int) fromX, (int) fromY, (int) toX, (int) toY);
    return;
  }

  
  // Do some quick Culls for both points way off to one side: right,
  // left, up, or down.  This will Cull MOST points out!!!!  Speed is
  // the result.

  //points all on right side,
  if((fromX >= widthPlus && toX >= widthPlus) ||
     //  on the left side,
     (fromX <= -lineWidthPlus1 && toX <= -lineWidthPlus1) ||
     //  on top (up),
     (fromY <= -lineWidthPlus1 && toY <= -lineWidthPlus1) ||
     // or on bottom (down)
     (fromY >= heightPlus && toY >= heightPlus))
    return; // Culled!!

  
  // Large value culling: We don't want to think about how large
  // numbers will act in the calculations below.  So we stop them
  // here.  LG is somewhere near the largest possible double (or
  // float).  Letting them through could cause a mess of lines all
  // over the window, that have nothing to do with the data.

  if(ABSVAL(fromX) > LG || ABSVAL(toX) > LG ||
     ABSVAL(fromY) > LG  || ABSVAL(toY) > LG ||
     ABSVAL(fromX-toX) > LG ||  ABSVAL(fromY-toY) > LG )
  {
    // This tipically does not happen, but to let the culling continue
    // with bad numbers may give bad results.
    
    if(opVerbose)
      opSpew << "Quickplot: large value culling: "
             << "from (" << fromX << ", " << fromY << ")   to ("
             << toX << ", " << toY << ")" << std::endl;
    return;
  }

  
  // now both points are not in the window (maybe one point is).
  // Below we refer to the line formed by the two points toX, toY and
  // fromX, formY as "line" or "the line"..
  
  if(toX != fromX)
  {
    value_t m = (toY - fromY)/(toX - fromX);
    if(ABSVAL(m) < ((value_t) 1.0e+6) && ABSVAL(m) > ((value_t) 1.0e-6))
    {
      // MOST COMMON line. // with both points not in the window.
      
      // This is a generic line: it's not close to horizontal,
      // horizontal, close to vertical, or vertical.
      value_t a = fromY - m * fromX;
      // the equation for the line is Y = a + m * X

      if(fromX > -lineWidthPlus1 && fromX < widthPlus &&
         fromY > -lineWidthPlus1 && fromY < heightPlus)
      {
        /************ one point in the window ***************/
        
        // point fromX, fromY is in the window and point toX, toY is
        // outside the window.

        // There is some overlap between these checks so that
        // floating point round off make us miss lines.

        if(toX > (value_t) width &&
           ((value_t)-1) < m*width+a &&
           m*width+a < ((value_t) height+1))
        {
          // crosses right side near X=width, Y=m*width+a
          if(toX > widthPlus) toX = widthPlus;
          
          (this->*drawLineFunc)((int) fromX, (int) fromY, (int) toX, (int) (m*toX+a));
          return;
        }
        if(toX < ZRO &&
           ((value_t)-1) < a &&
           a < ((value_t) height+1))
        {
          // crosses left side near X=0 at Y=a
          if(toX < -lineWidthPlus1) toX = -lineWidthPlus1;
         
          (this->*drawLineFunc)((int) fromX, (int) fromY, (int) toX, (int) (m*toX+a));
          return;
        }
        if(toY > (value_t) height &&
           ((value_t)-1) < (height -a)/m &&
           (height -a)/m < ((value_t) width+1))
        {
          // crosses bottom near Y=height,  X=(height -a)/m
          if(toY > heightPlus) toY = heightPlus;
          
          (this->*drawLineFunc)((int) fromX, (int) fromY, (int) ((toY -a)/m), (int) toY);
          return;
        }
        if(toY < ZRO &&
           ((value_t)-1) < -a/m &&
           -a/m < ((value_t) width+1))
        {
          // crosses top near Y=0,  X=-a/m
          if(toY < -lineWidthPlus1) toY = -lineWidthPlus1;
          
          (this->*drawLineFunc)((int) fromX, (int) fromY, (int) ((toY -a)/m), (int) toY);
          return;
        }

#if 0
        if(!opSilent)
          opSpew << "Quickplot: culling check"
                 << "from (" << fromX << ", " << fromY << ")   to ("
                 << toX << ", " << toY << ")" << std::endl
                 << "line=" << __LINE__ << " file=" << __FILE__
                 << std::endl;
#endif
        
        return; // Culled. Steep or shallow line near a window edge.
      }
      else if(toX > -lineWidthPlus1 && toX < widthPlus &&
              toY > -lineWidthPlus1 && toY < heightPlus)
      {
        /************ one point in the window ***************/
        
        // point toX, toY is in the window and point fromX, fromY is
        // not in the window

        // There is some overlap between these checks so that
        // floating point round off won't make us miss lines.

        if(fromX < ZRO &&
           ((value_t)-1) < a &&
           a < ((value_t) height+1))
        {
          // crosses left side near X=0, Y=a
          if(fromX < -lineWidthPlus1) fromX = -lineWidthPlus1;
         
          (this->*drawLineFunc)((int) fromX, (int) (m*fromX+a), (int) toX, (int) toY);
          return;
        }
        if(fromX > (value_t) width &&
           ((value_t)-1) < m*width+a &&
           m*width+a < ((value_t) height+1))
        {
          // crosses right side near X=width, Y=m*width+a
          if(fromX > widthPlus) fromX = widthPlus;
         
          (this->*drawLineFunc)((int) fromX, (int) (m*fromX+a), (int) toX, (int) toY);
          return;
        }
        if(fromY < ZRO &&
           ((value_t)-1) < -a/m &&
           -a/m < ((value_t) width+1))
        {
          // crosses top near Y=0, X=-a/m
          if(fromY < -lineWidthPlus1) fromY = -lineWidthPlus1;
         
          (this->*drawLineFunc)((int) ((fromY-a)/m), (int) fromY, (int) toX, (int) toY);
          return;
        }
        if(fromY > (value_t) height &&
           ((value_t)-1) < (height -a)/m &&
           (height -a)/m < ((value_t) width+1))
        {
          // crosses bottom near Y=height, X=(height -a)/m
          if(fromY > heightPlus) fromY = heightPlus;
          
          (this->*drawLineFunc)((int) ((fromY-a)/m), (int) fromY, (int) toX, (int) toY);
          return;
        }
#if 0
        if(!opSilent)
          opSpew << "Quickplot: culling check"
                 << "from (" << fromX << ", " << fromY << ")   to ("
                 << toX << ", " << toY << ")" << std::endl
                 << "line=" << __LINE__ << " file=" << __FILE__
                 << std::endl;
#endif
        
        return; // Culled. Steep or shallow line near a window edge
                // and outside the window.
      }
      else 
      {
        /************ No points in the window ***************/
        
        // a generic line with No points are in the window
        // Y = m*X + a     m is not large or small

        // We find the window intersection points with the greatest
        // distance appart along the line.  In most case there will be
        // just two points of intersection, but since we will be using
        // window edge lines that go 1 pixel past the edges to catch
        // lost points do to floating points round off, there may be
        // more than two points.  The most is 4 points.  Then we must
        // make sure that the two points toX, toY and fromX, fromY are
        // on oposite sides of the intersection points.

        // The four possible intersection points are.
        int icount=0; // point intersection count
        value_t X[4],Y[4]; // the array of 4  x,y points


        // We count and set the values as we find them.
        
        // x, y=0
        X[icount] = -a/m;// x
        if((value_t) -1 < X[icount] && X[icount] < (value_t) (width+1))
          Y[icount++] = ZRO; // y=0

        // x, y=height
        X[icount] = (height - a)/m;// x
        if((value_t) -1 < X[icount] && X[icount] < (value_t) (width+1))
          Y[icount++] = (value_t) height; // y=height

        // x=0, y
        Y[icount] = a; // y
        if((value_t) -1 < Y[icount] && Y[icount] < (value_t) (height+1))
          X[icount++] = ZRO; // x=0

        // x=width, y
        Y[icount] = width*m + a; // y
        if((value_t) -1 < Y[icount] && Y[icount] < (value_t) (height+1))
          X[icount++] = (value_t) width; // x=0

        if(icount <= 1)
          return; // Culled.  The line does not pass through the window.


        // icount == 2, 3 or 4
        // find the max in min x values.

        // I don't need to calulate distances like r^2 = delta_X^2 +
        // delta_Y^2, because all the points are on a straight line
        // that is not vertical or close to vertical.
        
        int max_i=0, min_i=0;
        int i;
        for(i=1;i<icount;i++)
        {
          if(X[i] > X[max_i])
            max_i = i;
          if(X[i] < X[min_i])
            min_i = i;
        }

        // The direction that the line is drawn may matter if the line
        // style shows the direction.  Like lines with arrows in them
        // like:  --->--->--->--- .
        
        if(fromX < toX)
        {
          // x is increasing along the line.
          (this->*drawLineFunc)((int) X[min_i], (int) Y[min_i],
                                (int) X[max_i], (int) Y[max_i]);
        }
        else 
        {
          // x is decreasing along the line ao draw the other
          // direction.
          (this->*drawLineFunc)((int) X[max_i], (int) Y[max_i],
                                (int) X[min_i], (int) Y[min_i]);
        }
        return;
      }
    }
    else if(ABSVAL(m) <= ((value_t) 1.2e-6) || toY == fromY)
      // overlaping some of the above case
    {
      // it's close to a horizontal line or is a horizontal line.

      // window middle Y value.  We assume that the window is much
      // less than 1e+6 (= 1,000,000) pixels across.
      
      value_t Y = fromY + m * (width/2 - fromX);

      if(fromX > (value_t) width)
        fromX = (value_t) width;
      else if(fromX < ZRO)
        fromX = ZRO;

      if(toX > (value_t) width)
        toX = (value_t) width;
      else if(toX < ZRO)
        toX = ZRO;

      //opSpew << "close to horizontal" << std::endl;
      
      if(fromX != toX)
        (this->*drawLineFunc)((int) fromX, (int) Y, (int) toX, (int) Y);
      return;
    }
    else // toY != fromY && slope > 1e+6 && toX != fromX
    {
      // it's Not: close to horizontal, horizontal, generic, or vertical.
      
      // it's close to a vertical line   by process of elimination
      
      // try m = recipical slope because it's close to a verticle
      // line.  And given the Culling above there must be a line
      // some where in the window.
      
      m = (toX - fromX)/(toY - fromY);

      // middle X value.
      value_t X = fromX + m * (height/2 - fromY);

      if(fromY > (value_t) height)
        fromY = (value_t) height;
      else if(fromY < ZRO)
        fromY = ZRO;

      if(toY > (value_t) height)
        toY = (value_t) height;
      else if(toY < ZRO)
        toY = ZRO;

      //opSpew << "close to vertical" << std::endl;

      if(fromY != toY)
        (this->*drawLineFunc)((int) X, (int) fromY, (int) X, (int) toY);
      return;
    }
  }
  else // toX == fromX
  {
    // It's a verticle line

    // It must be near the window, because it has not been culled out above.

    if(fromY > (value_t) height)
      fromY = (value_t) height;
    else if(fromY < ZRO)
      fromY = ZRO;

    if(toY > (value_t) height)
      toY = (value_t) height;
    else if(toY < ZRO)
      toY = ZRO;

    //opSpew << "vertical" << std::endl;

    if(fromY != toY) // if not both to top or bottom (just on the edge).
      (this->*drawLineFunc)((int) fromX, (int) fromY, (int) toX, (int) toY);
    return;
  }
    
#endif //#ifdef WITHOUT_CULLING  #else
}
