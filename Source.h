/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */

#define STDIN_FILENAME  "<standard input>"

class Field;
class Source;
class MainWindow;

extern "C"
{
  struct SourceDeleteLater
  {
    Source *source;
  };
}

class CloseSourceMenuItem : public ImageMenuItem
{
public:
  CloseSourceMenuItem(Source *s, MainWindow *mainWindow_in);
  void deleteSourceLater(void);

private:
  
  Image closeImage;

  struct SourceDeleteLater dl;
  
public:

  MainWindow *mainWindow;
};



class Source : public std::list<Field *>
{
public:
  
  Source(void);
  virtual ~Source(void);


  bool isValid;

  enum TYPE { NONE=0, SNDFILE, ASCII_FILE };

 
  inline const enum  TYPE getType(void){ return type; }
  inline const char *getTypeString(void){ return TYPE_STRING[type]; }

  static SigC::Signal1<void, Source *> signal_addedSource();
  static SigC::Signal1<void, Source *> signal_removedSource();

  // Delete this object later.
  void deleteLater(void);

  std::list<CloseSourceMenuItem *> closeSourceMenuItems;
  
  
protected:
  
  static SigC::Signal1<void, Source *> m_signal_addedSource;
  static SigC::Signal1<void, Source *> m_signal_removedSource;

  inline void setType(enum TYPE t) { type = t; }
  char *fileName, *baseFileName;
  void addCloseMenus(const char *label);

private:

  TYPE type;
  
  static const char *TYPE_STRING[];
  
  
public:
  
  inline const char *getFileName(void) { return fileName; }
  inline const char *getBaseFileName(void) { return baseFileName; }
 
};

extern std::list<Source *> sources;
