/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */


// This is a fgetc() wrapper that lets me have an effectively have
// rewind() for the standard input file, stdin.


#define CHUNK  ((size_t) 64)



class FileReader
{
 public:

  FileReader(const char *filename, FILE *file=NULL);
  ~FileReader(void);

  int readChar(void); // like fgetc()


  void rewind(void); // like rewind().
  
  // stops saving the data read, so you may not call
  // FileReader::rewind() after this.
  void freeRewind(void);

  int endOfFile;

 private:

  FILE *file;
  unsigned char *buffer;
  size_t bufferSize;

  size_t in;  // in from file
  size_t out; // out to user of this class
  int isFree;
  int openedFile;
  int _endOfFile;
};

