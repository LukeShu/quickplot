/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */

#include <string.h>

// Get an option argument at index *i;
// if found index the counter *i to the next string.

char *getOpt(const char *shorT, const char *lonG,
	     int argc, const char **argv, int *i)
{
  // check for form long=VALUE
  size_t len = 0;
  if(*i < argc)
    {
      char *str = (char *) argv[*i];
      for(;*str && *str != '=';str++);
      if(*str == '=')
        {
          len = (size_t) str - (size_t) argv[*i];
          str++;
        }
      if(len && !strncmp(argv[*i],lonG,len)
          &&  *str != '\0')
        {
          (*i)++;
          return str;
        }
    }

  // check for  -a VALUE   or --long VALUE
  if(((*i + 1) < argc) &&
     (!strcmp(argv[*i],lonG) || !strcmp(argv[*i],shorT)))
    {
      (*i)++;
      return (char *) argv[(*i)++];
    }

  // check for   -aVALUE
  len=strlen(shorT);
  if( (*i < argc) && !strncmp(argv[*i],shorT,len) && argv[*i][len] != '\0')
    return (char *) &(argv[(*i)++][len]);


  return (char *) NULL;
}
