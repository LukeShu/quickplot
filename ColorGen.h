/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */



#define NUM_COLORS  12

class ColorGen // keeps a list of colors and cycles through them.
{
public:

  ColorGen(void);

  // get line and point colors.
  void getColors(gushort &lr, gushort &lg, gushort &lb,
                 gushort &pr, gushort &pg, gushort &pb);

  void getGridColor(gushort &r, gushort &g, gushort &b);

  void getBackgroundColor(gushort &r, gushort &g, gushort &b);
  
  static gushort gridColor[3];
  static gushort backgroundColor[3];

  
private:

  unsigned int count;

  static const gushort color[NUM_COLORS][3];
};
