/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */

/* class LogField: takes the log of the field passed to it.  The log
 * is computed at the time of the read() call.  This does not store
 * the field data in the LogField objects data, but just reads it from
 * the Field object that was passed to it. The Field object that is
 * passed to the constructor must stay valid for this to work.
 *
 *
 * The function log(x) is not valid for x <= 0.  So we need an error
 * check.
 *
 */


class LogField : public Field
{
 public:

  // value = count * scale + offset
  inline LogField(Source *s, Field *f, value_t base=(value_t)10.0):
    Field(s), 
    {
      error = 0; // start with no error.
      
      here lanceman
        
      _isIncreasing = (scale > 0.0)? true: false;
      _numberOfValues = MAXCOUNT;
      _max = (_numberOfValues-1)*scale + offset;
      _min = offset;
      // assuming that count_t is unsigned so that -1 is the maxmum
      // value
      index = 0;

      setName("linear field");
      
    }
  
  // read the list and advance the pointer to the next value in the
  // series.
  inline value_t read(void)
    {
      return scale*(index++) + offset;
    }

  // find the minimum values in the range of indexes
  // indexing starts at 0.
  inline value_t max(count_t i_max=((count_t) -1), count_t i_min=0)
    {
      if(i_max == ((count_t) -1))
        i_max = _numberOfValues - 1;
      if(scale > (value_t) 0.0)
        return scale*(i_max) + offset;
      else
        return scale*(i_min) + offset;
    }
  
  // find the maximum values in the range of indexes
  // indexing starts at 0.
  inline value_t min(count_t i_max=((count_t) -1), count_t i_min=0)
    {
      if(i_max == ((count_t) -1))
        i_max = _numberOfValues - 1;
      if(scale < (value_t) 0.0)
        return scale*(i_max) + offset;
      else
        return scale*(i_min) + offset;
    }

  // move the pointer to the first value.
  inline void rewind(void) { index = 0; }
  
private:
  
  // Index to the element to be read.
  value_t base;
  
};
