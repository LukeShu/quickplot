/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */
#include "config.h"

#include <stdio.h>
#include <stdlib.h>

// w and h will not be set if the width and height part of the
// geometry are not valid.  x and y will not be set if the x-position
// and y-position part of the geometry are not valid.
// The values for xSign and ySign may change.

// example:
//  -geometry  800x240+-30-200     w x h xSign x ySign y
//     ====> w=800 h=240 xSign=1 x=-30 ySign=-1 y=200


void parseGeometry(const char *geometry,
                   int &w, int &h,
                   int &x, int &y,
                   int &xSign, int & ySign)
{
  // If the geometry string is longer than 32 is won't work on any
  // display I've ever heard of.

  //  example: -geometry  800x240+30+200
  
  //  example: -geometry  800  x  240  +  300  +  200 \0
  //            geo=      WWW \0  HHH \0       Y  YYY \0
  //           xgeo=                   X  XXX \0
  
  // or even like: -geometry  800x240--30+-200
  
  char geo[32];
  char xgeo[32];
  snprintf(geo, 32, "%s", geometry);
  snprintf(xgeo, 32, "%s", geometry);
  
  char *W=NULL, *H=NULL, *X=NULL, *Y=NULL;
  
  char *s;
  
  // Look for W width and H height WxH
  s = geo;
  for(;*s != 'x' && *s != 'X' && *s != '\0'; s++)
    ;
  if(*s == 'x' || *s == 'X')
  {
    W = geo;
    *s = '\0'; // replace the 'x' || 'X' with '\0'.
    H = ++s;
    for(;*s != '-' && *s != '+' && *s != '\0'; s++)
      ;
    
    if(*s == '-' || *s == '+')
    {
      *s = '\0';
      s++;
    }
  }
  
  if(W && H && W[0] && H[0])
  {
    // resize the window based on W and H
    int w_ = atoi(W);
    int h_ = atoi(H);
    
    if(w_ >= 0 && w_ < 100000 && h_ >=0 && h_ < 100000)
    {
      w = w_;
      h = h_;
    }
  }

  // Look for Y position +Y or -Y
  if(W)
  {
    if(*s == '-' || *s == '+')
      s++;
    
    for(;*s != '-' && *s != '+' && *s != '\0'; s++)
      ;
    if(*s == '-' || *s == '+')
    {
      ySign = (*s == '+')? 1: -1;
      s++;
      Y = s;
    }
  }
  else // !W
  {
    s = geo;
    for(;*s != '-' && *s != '+' && *s != '\0'; s++)
      ;
    s++;
    if(*s == '-' || *s == '+')
      s++;
    
    for(;*s != '-' && *s != '+' && *s != '\0'; s++)
      ;
    if(*s == '-' || *s == '+')
    {
      ySign = (*s == '+')? 1: -1;
      s++;
      Y = s;
    }
  }
  
  
  // Look for X position +X or -X
  if(Y)
  {
    s = xgeo;
    for(;*s != '-' && *s != '+' && *s != '\0'; s++)
        ;      
    if(*s == '-' || *s == '+')
    {
      xSign = (*s == '+')? 1: -1;
      s++;
      X = s;
      s++;
      for(;*s != '-' && *s != '+' && *s != '\0'; s++)
        ;
      if(*s == '-' || *s == '+')
      {
        *s = '\0'; // replace the '+' || '-' with '\0'.
      }
      else
      {
        X = NULL;
        Y = NULL;
      }
    }
  }
  
  
  if(X && Y && X[0] && Y[0])
  {
    // Get x, y, xSign, and ySign and use in the
    // first call to on_expose_event().
    int x_ = atoi(X);
    int y_ = atoi(Y);
    
    if(x_ > -100000 && x_ < 100000 &&
       y_ > -100000 && y_ < 100000)
    {
      x = x_;
      y = y_;
    }
  }
}
