/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */
class Source;
class FieldReader;

class Field
{
 public:

  Field(Source *s);
  virtual ~Field(void);

  // The source (file) of the field data.
  Source *source;

  // read the list and advance the pointer to the next value in the
  // series.
  virtual value_t read(void) = 0;

  // move the pointer to the first value in the series.
  virtual void rewind(void) = 0;

  virtual inline count_t numberOfValues(void) { return _numberOfValues; }
  
  // append the value to the end of the list.
  // void write(T value); is not in all Field classes
  

  // Find the minimum values in the range of indexes. Indexing starts
  // at 0.  i_max=-1 means upto the numberOfValues.
  virtual value_t max(count_t i_max=((count_t) -1), count_t i_min=0);
  
  // find the minimum values in the range of indexes.  Indexing starts
  // at 0.  i_max=-1 means upto the numberOfValues.
  virtual value_t min(count_t i_max=((count_t) -1), count_t i_min=0);

  // Is monotonically increasing, like time.
  virtual inline bool isIncreasing(void) { return _isIncreasing; }
  virtual inline bool isDecreasing(void) { return _isDecreasing; }

  void setName(const char *name_in);
  void setLabel(const char *label_in);
  inline const char *getName(void) { return name; }
  inline const char *getLabel(void) { return label; }

protected:

  // A derived Field class must maintain these numbers or over-write
  // the corresponding virtual method.  This data is waste in a
  // FieldReader.
  value_t _min, _max;
  count_t _numberOfValues;
  bool _isIncreasing;
  bool _isDecreasing;

  
public:

  friend class FieldReader;
  // The class FieldReader is a Field that can read() another Field
  // (the source Field) without effecting the reading by the methods,
  // read(void), and rewind(void) of the source Field object that it
  // is reading with.  The FieldReader makes it so that you can have
  // more than one object read the data in a Field.  This saves having
  // to have more than one copy of the data in memory.  An example
  // when this is needed is when your wish to read two fields which
  // are derived from the same series of numbers.  The two Fields can
  // share the data if one (or both) of them is a FieldReader.

  // The Field just needs to implement these three methods for the
  // FieldReader to Field interface.  Then any Field can be a source
  // for a FieldReader. Even a FieldReader can be a source for a .
  
  // makeDequeuer sets up so that another object can read from the
  // data in this Field.  Don't forget to clean up in the destructor.
  // One must be sure that the dequeuer pointer is not pointing to a
  // C++/class like thingy, because casing will break it. If in doubt
  // C-ify it with extern "C" {}.
  virtual void *makeDequeuer(count_t number_of_values) = 0;
  virtual void destroyDequeuer(void *dequeuer) = 0;
 
  // The FieldReader object can use this to read.
  virtual value_t read(void *dequeuer) = 0;

  // read backwards through the list.
  virtual value_t readBack(void *dequeuer) = 0;

  // The FieldReader object can use this to rewind.
  virtual void rewind(void *dequeuer) = 0;

  
private:
  
  // fieldReader is the list of FieldReaders that are using this
  // Field.  FieldReaders in the list remove themselfs in
  // FieldReader::~FieldReader().  FieldReader::FieldReader() adds
  // them to this list.
  FieldReader *fieldReader;
  
  char *name;
  char *label;
};
