/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define TMP_FORMAT  "%s/Quickplot_%s_XXXXXXXX"


char *browsers[] =
{
  "galeon",
  "konqueror",
  "mozilla",
  "epiphany",
  "netscape",
  "dillo",
  /*
  "amaya",
  "browsex",
  "firefox",
  "light",
  */
  NULL
};
  

int launchBrowser(const char *url)
{
  pid_t pid = vfork();
  if(pid < 0) //error
  {
    perror("Quickplot ERROR: vfork() failed");
    return -1;
  }
  if(pid > 0) // parent
    return 0;

  // I'm the child.

  // First see if the user has set a prefered broswer in the BROWSER
  // environment variable.
  char *browser = getenv("BROWSER");

  if(browser)
    execlp(browser, browser, url, NULL);

  // now try the list of browsers
  int i=0;
  for(browser=browsers[i]; browser; browser=browsers[++i])
    execlp(browser, browser, url, NULL);
  
  exit(1);
  
  return 0; //success
}

int sLaunchBrowser(const char *doc, const char *fileName_in="")
{
  size_t len =
    strlen(HTMLDIR)+
    L_tmpnam+
    ((fileName_in)?strlen(fileName_in):0)+
    strlen(TMP_FORMAT);
  
  char *filename = (char *) malloc(sizeof(char)*len);
  filename[0] = '\0';

  if(fileName_in && fileName_in[0])
  {
    // See if we can find the html file in HTMLDIR the doc
    // installation directory.
    snprintf(filename, len, "%s/%s", HTMLDIR, fileName_in);
  
    // first we try to stat the installed html file filename.
    struct stat buf;
    int rt = stat(filename, &buf);
    if(!(rt == 0 && (S_ISREG(buf.st_mode) || S_ISLNK(buf.st_mode)))) 
    {
      // So much for that idea.
      filename[0] = '\0';
    }
  }

  if(!(filename[0]))
  {
      
    // installed docs won't work. Make a tmp file.
    snprintf(filename, len, TMP_FORMAT, P_tmpdir, fileName_in);
  
    int fd = mkstemp(filename);
    if(fd < 0)
    {
      // We can't make a temp file.  we're screwed now.
      perror("Quickplot ERROR: mkstemp() failed");
      free(filename);
      return -1;
    }
    
    write(fd, doc, strlen(doc)+1);
    close(fd);
  }

  launchBrowser(filename);
  
  free(filename);
  return 0; //success
}


//#define test
#ifdef test

int main(int argc, char **argv)
{
  if(argc < 2)
  {
    const char *src =
      "<html>\n"
      "<head>\n"
      "<title>Test Page</title>\n"
      "</head>\n"
      "<body>\n"
      "Hello World.\n"
      "</body>\n";
    
    printf("sLaunchBrowser()=%d\n", sLaunchBrowser(src));
  }
  else
  {
    printf("launchBrowser(\"%s\")=%d\n",
           argv[1], launchBrowser(argv[1]));
  }

  return 0;
}
#endif
