/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */

// This is the type that is used to scale values as they are plotted
// though they may be stored as a different type in the Field.

#define QUICKPLOT_USE_DOUBLE 1

#if QUICKPLOT_USE_DOUBLE
  typedef double         value_t;

#if defined MINGW || defined QP_ARCH_DARWIN
#  define MAXDOUBLE 1.0e+302
#  define MINDOUBLE 1.0e-302
  typedef long long int64_t;
#endif


#  define MAXVALUE       MAXDOUBLE
#  define MINVALUE       MINDOUBLE

#  define STRINGTOVALUE  strtod
#  define VALUESTRING    "double"
#  define GRID_PRINT_FORMAT     "%.15g"
#  define FORMAT                "%.12g"
#  define TWO_DIGIT_FORMAT      "%.2g"
#  define SCAN_FORMAT           "%lf"

// Math C functions for this type
#  define SQRT           sqrt
#  define LOG10          log10
#  define POW            pow


#else /* #if QUICKPLOT_USE_DOUBLE */

  typedef float         value_t;

#  define MAXVALUE       MAXFLOAT
#  define MINVALUE       MINFLOAT

#  define STRINGTOVALUE  strtof
#  define VALUESTRING    "float"
#  define GRID_PRINT_FORMAT     "%.8g"
#  define FORMAT                "%.8g"
#  define TWO_DIGIT_FORMAT      "%.2g"
#  define SCAN_FORMAT           "%f"

// Math C functions for this type
#  define SQRT           sqrtf
#  define LOG10          log10f
#  define POW            powf

#endif /* #else  #if QUICKPLOT_USE_DOUBLE */


// This is the type that is used to count the values as they are
// plotted.
typedef unsigned long   count_t;

#define MAXCOUNT       ((count_t) -1)
