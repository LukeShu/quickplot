/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */

// This is just to keep a list of filenames and there associated
// options.  This is just used in App_parseArgs.cpp.



class FileList
{
 public:

  FileList(char *filename=NULL);
  ~FileList(void);

  void setFileName(const char *filename);
  inline char *getFileName(void) const
    {
      return fileName;
    }

  // ASCII file options, ignored for other file types.
  count_t skipLines;
  bool readLabels;
  char labelSeparator;

  bool hasLinearField;
  value_t linearFieldStep, linearFieldStart;
  bool takeLog;

  FileList *next;
  static FileList *first;

 private:

  char *fileName;

};
