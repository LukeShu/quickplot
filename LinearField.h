/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */
class Source;
class Field;

class LinearField : public Field
{
 public:

  // value = count * step + offset
  inline LinearField(Source *source,
                     value_t step_in=(value_t)1.0,
                     value_t offset_in=(value_t)0.0):
    Field(source),
    step(step_in), offset(offset_in)
    {
      _isIncreasing = (step > 0.0)? true: false;
      _numberOfValues = MAXCOUNT;
      _max = (_numberOfValues-1)*step + offset;
      _min = offset;
      // assuming that count_t is unsigned so that -1 is the maxmum
      // value
      index = 0;

      setLabel("linear field");
      char label[64];
      snprintf(label, 64, "step=%g offset=%g", step, offset);
      setName(label);
    }
  
  // read the list and advance the pointer to the next value in the
  // series.
  inline value_t read(void)
    {
      return step*(index++) + offset;
    }

  // find the minimum values in the range of indexes
  // indexing starts at 0.
  inline value_t max(count_t i_max=((count_t) -1), count_t i_min=0)
    {
      if(i_max == ((count_t) -1))
        i_max = _numberOfValues - 1;
      if(step > (value_t) 0.0)
        return step*(i_max) + offset;
      else
        return step*(i_min) + offset;
    }
  
  // find the maximum values in the range of indexes
  // indexing starts at 0.
  inline value_t min(count_t i_max=((count_t) -1), count_t i_min=0)
    {
      if(i_max == ((count_t) -1))
        i_max = _numberOfValues - 1;
      if(step < (value_t) 0.0)
        return step*(i_max) + offset;
      else
        return step*(i_min) + offset;
    }

  // move the pointer to the first value.
  inline void rewind(void) { index = 0; }

  struct Dequeuer
  {
    count_t index, numberOfValuesMinus1;
  };

  void *makeDequeuer(count_t number_of_values);
  void destroyDequeuer(void *dequeuer);

  
  // You can use this to read.
  inline value_t read(void *dequeuer)
    {
      struct Dequeuer *d = (struct Dequeuer *) dequeuer;
      value_t val = step*(d->index) + offset;
      if(d->index < d->numberOfValuesMinus1)
        (d->index)++;
      return val;
    }

  // You can use this to read backwards.
  inline value_t readBack(void *dequeuer)
    {
      struct Dequeuer *d = (struct Dequeuer *) dequeuer;
      value_t val = step*(d->index) + offset;
      if(d->index > 0)
        (d->index)--;
      return val;
    }


  // The FieldReader object can use this to rewind.
  inline void rewind(void *dequeuer)
    {
      ((struct Dequeuer *) dequeuer)->index = 0;
    }
  
private:
  
  // Index to the element to be read.
  count_t index;
  value_t step, offset;

};
