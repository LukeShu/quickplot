/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */

#include <stdlib.h>
#include <errno.h>
#include <string.h>

/* The data type the quickplot uses to plot is a kind of float set in
 * value_t.h.  The type that the Field stores the values as can be any
 * type, which may save memory in most cases.
 */
template<class T> class ArrayField : public Field
{
 public:

  ArrayField(Source *source, count_t numberOfValues);
  virtual ~ArrayField(void);

  // read the list and advance the pointer to the next value in the
  // series.
  inline value_t read(void)
    {
      return static_cast<value_t>(value[index++]);
    }
  
  // find the maximum values in the range of indexes
  // indexing starts at 0.
  inline value_t max(count_t i_max=((count_t) -1), count_t i_min=0)
    {
      value_t Max = -MAXVALUE;
      if(i_max == ((count_t) -1))
        i_max = _numberOfValues - 1;
      count_t i;
      for(i=i_min;i<=i_max;i++)
        if(value[i] > Max)
          Max = value[i];
      return Max;
    }
  
  // find the minimum values in the range of indexes
  // indexing starts at 0.
  inline value_t min(count_t i_max=((count_t) -1), count_t i_min=0)
    {
      value_t Min = MAXVALUE;
      if(i_max == ((count_t) -1))
        i_max = _numberOfValues - 1;
      count_t i;
      for(i=i_min;i<=i_max;i++)
        if(value[i] < Min)
          Min = value[i];
      return Min;
    }

  // move the pointer to the first value.
  inline void rewind(void) { index = 0; }

  void write(T t);

  struct Dequeuer
  {
    count_t index, numberOfValuesMinus1;
  };

  
  void *makeDequeuer(count_t number_of_values);
  void destroyDequeuer(void *dequeuer);

  // The FieldReader object can use this to read.
  inline value_t read(void *dequeuer)
    {
      struct Dequeuer *d = (struct Dequeuer *) dequeuer;
      value_t val = static_cast<value_t>(value[d->index]);
      if(d->index < d->numberOfValuesMinus1)
        (d->index)++;
      return val;
    }
  
  // read backwards through the list.
  virtual value_t readBack(void *dequeuer)
    {
     struct Dequeuer *d = (struct Dequeuer *) dequeuer;
     value_t val = static_cast<value_t>(value[d->index]);
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
  
  // Index to the element to be read or written.
  count_t index;
  T *value;
};

template<class T>
ArrayField<T>::ArrayField(Source *source, count_t NumberOfValues):
  Field(source)
{
  value = static_cast<T*>(malloc(sizeof(T) * NumberOfValues));
  if(!value)
  {
    snprintf(errorStr, ERRORSTR_LENGTH,
             "Quickplot ERROR: malloc(%ld) failed: "
             "system error number %d: %s",
             sizeof(T) * NumberOfValues, errno, strerror(errno));
    error = 1;
  }
  // We assume that this many values will be written.
  _numberOfValues = NumberOfValues;
  index = 0;
}


template<class T>
ArrayField<T>::~ArrayField(void)
{
  if(value)
    free(value);
}


template<class T>
void ArrayField<T>::write(T x_in)
{
  // You can't write more than _numberOfValues values.
  value[index++] = x_in;
  
  // assuming T is not the same type as value_t
  value_t x = x_in; // convert to value type.
  if(x > _max)
    _max = x;
  else if(_isIncreasing)
    _isIncreasing = false;

  if(x < _min)
    _min = x;
  else if(_isDecreasing)
    _isDecreasing = false;
}

template<class T>
void *ArrayField<T>::makeDequeuer(count_t number_of_values)
{
  struct Dequeuer *d = (struct Dequeuer *) malloc(sizeof(struct Dequeuer));
  d->numberOfValuesMinus1 =
    ((number_of_values<_numberOfValues)?number_of_values:_numberOfValues)
    -1;
  d->index = 0;
  return (void *) d;
}

template<class T>
void ArrayField<T>::destroyDequeuer(void *dequeuer)
{
  free(dequeuer);
}
