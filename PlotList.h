/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */


class PlotList
{
 public:

  PlotList(void);
  ~PlotList(void);


  void add(int X, int Y);
  void rewind(void);
  void get(int &X, int &Y);
  inline int getNumberOfPlots(void)
    {
      return numPlots;
    }

  PlotList *next;
  static PlotList *first;

private:

  // field number starting at 0.
  int *x, *y;
  
  size_t numPlots;
  int index;
};
