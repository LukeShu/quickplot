/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */

// The class FileReader is a Field that can read() another Field (the
// source Field) without effecting the reading by the methods,
// read(void), and rewind(void) of the source Field object that it is
// reading with.  The FileReader makes it so that you can have more
// than one object read the data in a Field.  This saves having to
// have more than one copy of the data in memory.  An example when
// this is needed is when your wish to read two fields which are
// derived from the same series of numbers.  The two Fields can share
// the data if one (or both) of them is a FieldReader.  This is some
// kind of clone of the source Field, without the memory for the data
// in the Field.  This saves memory.  We do this stuff because there
// are many different kinds of fields which store and retrieve the
// series (Field) data in many ways.

class Field;

class FieldReader : public Field
{
public:

  FieldReader(Field *f);
  ~FieldReader(void);
      

  // read the list and advance the pointer to the next value in the
  // series.
  virtual inline value_t read(void)
    {
      return field->read(dequeuer);
    }

  // move the pointer to the first value in the series.
  virtual inline void rewind(void)
    {
      field->rewind(dequeuer);
    }

  virtual inline count_t numberOfValues(void) { return field->numberOfValues(); }
  
  // append the value to the end of the list.
  // virtual write(T value);
  

  // Find the minimum values in the range of indexes. Indexing starts
  // at 0.  i_max=-1 means upto the numberOfValues.
  virtual inline value_t max(count_t i_max=((count_t) -1), count_t i_min=0)
    {
      return field->max(i_max, i_min);
    }
  
  // find the minimum values in the range of indexes.  Indexing starts
  // at 0.  i_max=-1 means upto the numberOfValues.
  virtual inline value_t min(count_t i_max=((count_t) -1), count_t i_min=0)
    {
      return field->min(i_max, i_min);
    }

  
  // Is monotonically increasing, like time.
  virtual inline bool isIncreasing(void) { return field->isIncreasing(); }
  virtual inline bool isDecreasing(void) { return field->isDecreasing(); }

  virtual void *makeDequeuer(count_t NumberOfValues);
  virtual void destroyDequeuer(void *dequeuer);

  // The FieldReader object can use this to read.
  virtual inline value_t read(void *dequeuer)
    {
      return field->read(dequeuer);
    }

  // The FieldReader object can use this to rewind.
  virtual inline void rewind(void *dequeuer)
    {
      field->rewind(dequeuer);
    }


private:

  friend class Field;

  // next is used in a linked list of FieldReaders is kept in the
  // source Field object in Field::fieldReader.
  FieldReader *next;

  void *dequeuer;

  // The data source for this Field.
  Field *field;
};
