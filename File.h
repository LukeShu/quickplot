/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */


struct ::SNDFILE;
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
  
  // This will not work with stdin.
  bool readSndFile(const FileList *fileList, int fd=-1);

  void readSndShort  (::SNDFILE *sf, int samplerate,
                      count_t numberOfValues, int channels,
                      const FileList *fileList);
  void readSndInt    (::SNDFILE *sf, int samplerate,
                      count_t numberOfValues, int channels,
                      const FileList *fileList);
  void readSndFloat  (::SNDFILE *sf, int samplerate,
                      count_t numberOfValues, int channels,
                      const FileList *fileList);
  void readSndDouble (::SNDFILE *sf, int samplerate,
                      count_t numberOfValues, int channels,
                      const FileList *fileList);

  // This will work with stdin.
  bool readASCIIFile(FILE *file, const FileList *fileList);

  int getValue(FileReader *file, value_t *val);
};
