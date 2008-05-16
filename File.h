/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */
// For amd64. Why in here I don't know.
#ifdef USE_LIBSNDFILE
#include <sndfile.h>
#endif

class Source;
class FileReader;
class FileList;

class File : public Source
{

public:

  // if filename=NULL then use stdin
  File(const FileList *fileList);
  File(const char *filename);
  
  
private:

  void init(const FileList *fileList);
  
#ifdef USE_LIBSNDFILE

  // This will not work with stdin.
  bool readSndFile(const FileList *fileList, int fd=-1);

  void readSndShort  (::SNDFILE *sf, int samplerate,
                      count_t numberOfValues, int channels);
  void readSndInt    (::SNDFILE *sf, int samplerate,
                      count_t numberOfValues, int channels);
  void readSndFloat  (::SNDFILE *sf, int samplerate,
                      count_t numberOfValues, int channels);
  void readSndDouble (::SNDFILE *sf, int samplerate,
                      count_t numberOfValues, int channels);

#endif // #ifdef USE_LIBSNDFILE


  // This will work with stdin.
  bool readASCIIFile(FILE *file, const FileList *fileList);

  int getValue(FileReader *file, value_t *val);
};
