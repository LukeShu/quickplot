/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */

/* The data type the quickplot uses to plot is a kind of float set in
 * value_t.h.  The type that the Field stores the values as can be any
 * type, which may save memory in most cases.
 */

/* Class ListField: stores the data in a linked list.  Can be used to
 * store fields that can change the number of elements (data points).
 */

template<class T> struct DataList 
{
  struct DataList<T> *next, *prev;
  T value;
};


template<class T> class ListField : public Field
{
 public:

  ListField(Source *source);
  virtual ~ListField(void);

  // read the list and advance the pointer to the next value in the
  // series.
  inline value_t read(void)
    {
       value_t value = current->value;
      if(current->next)
        current = current->next;
      return value;
    }
  
  // move the pointer to the first value.
  inline void rewind(void) { current = start; }

  void write(T t);

  struct Dequeuer
  {
    count_t index, numberOfValuesMinus1;
    struct DataList<T> *current;
  };
  
  void *makeDequeuer(count_t number_of_values);
  void destroyDequeuer(void *dequeuer);

  // The FieldReader object can use this to read.  The dequeuer is a
  // handle to a struct DataList<T>.
  inline value_t read(void *dequeuer)
    {
      struct Dequeuer *d = (struct Dequeuer *) dequeuer;
      value_t val = d->current->value;
     
      if(d->index < d->numberOfValuesMinus1)
      {
        d->current = d->current->next;
        (d->index)++;
      }
      
      return val;
    }

  inline value_t readBack(void *dequeuer)
    {
      struct Dequeuer *d = (struct Dequeuer *) dequeuer;
      value_t val = d->current->value;
     
      if(d->index > 0)
      {
        d->current = d->current->prev;
        (d->index)--;
      }
      
      return val;
    }

  // The FieldReader object can use this to rewind.
  inline void rewind(void *dequeuer)
    {
      ((struct Dequeuer *) dequeuer)->current = start;
    }

  
  // next is a Kludge used in File_readASCII() to access this field
  // quickly when writing the data.
  ListField<T> *next;

private:
  
  // Index to the element to be read or written.
  struct DataList<T> *start, *current, *end;
};

template<class T>
ListField<T>::ListField(Source *source):
  Field(source)
{
  // We assume that this many values will be written.
  _numberOfValues = 0;
  start = current = end = NULL;
}

template<class T>
ListField<T>::~ListField(void)
{
  current = start;
  while(current)
  {
    start = current->next;
    free(current);
    current = start;
  }
}


template<class T>
void ListField<T>::write(T x)
{
  struct DataList<T> *d = (struct DataList<T> *)
    malloc(sizeof(struct DataList<T>));

  d->next = NULL;
  d->value = x;
  
  if(!start)
  {
    current = start = end = d;
    d->prev = NULL;
  }
  else
  {
    end->next = d;
    d->prev = end;
    end = d;
  }

  if(x > _max)
    _max = x;
  else if(_isIncreasing)
    _isIncreasing = false;

  if(x < _min)
    _min = x;
  else if(_isDecreasing)
    _isDecreasing = false;

  _numberOfValues++;
}

template<class T>
void *ListField<T>::makeDequeuer(count_t number_of_values)
{
  struct Dequeuer *d = (struct Dequeuer *) malloc(sizeof(struct Dequeuer));
  d->numberOfValuesMinus1 =
    ((number_of_values<_numberOfValues)?number_of_values:_numberOfValues)
    -1;
  d->index = 0;
  d->current = start;
  
  return (void *) d;
}

template<class T>
void ListField<T>::destroyDequeuer(void *dequeuer)
{
  free(dequeuer);
}
