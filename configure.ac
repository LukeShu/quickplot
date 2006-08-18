# This file is for people developing quickplot. This file is used to
# generate many other files using the GNU autotools. You do this by
# running `./bootstrap'.  If you're not developing or hacking
# quickplot you likely don't want to run `./bootstrap' or
# `./cvsclean'.

# require autoconf version
AC_PREREQ(2.58)

##########################################################
############ Configure a release here ####################
##########################################################

# AC_INIT( PACKAGE, VERSION, PACKAGE_BUGREPORT, TARNAME )

AC_INIT([Quickplot],
 0.8.11,
 [lanceman@users.sourceforge.net],
 quickplot)

RELEASE_DATE="Thu Aug 17 23:00:57 EDT 2006"
HOMEPAGE="http://quickplot.sourceforge.net/"


##########################################################
######## END: Configure a release here ####################
##########################################################


#AM_INIT_AUTOMAKE(option1 option2 ...)
AM_INIT_AUTOMAKE(dist-bzip2)

AM_CONFIG_HEADER(config.h)


# Checks for programs.
missing_dir=`cd $ac_aux_dir && pwd`
AM_MISSING_PROG(ACLOCAL, aclocal, $missing_dir)
AM_MISSING_PROG(AUTOCONF, autoconf, $missing_dir)
AM_MISSING_PROG(AUTOMAKE, automake, $missing_dir)
AM_MISSING_PROG(AUTOHEADER, autoheader, $missing_dir)
AM_MISSING_PROG(MAKEINFO, makeinfo, $missing_dir)


AC_CONFIG_SRCDIR([config.h.in])


AC_PROG_CXX
#AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
#AC_PROG_LN_S
AC_PROG_MAKE_SET
#AC_PROG_RANLIB

# `info autoconf' says use this order
# -- checks for types
# -- checks for structures
# -- checks for compiler characteristics
# -- checks for library functions
# -- checks for system services

##########################################################

ARCH="`uname`"

#These three we know so far
# Linux
# IRIX or IRIX64
# Darwin

case "${ARCH}" in

  Darwin)
    AC_DEFINE(QP_ARCH_DARWIN, [],
   [The OS or architecture which was output\
 from `uname' when configure ran.])
  ;;

  Linux)
  AC_DEFINE(QP_ARCH_LINUX, [],
   [The OS or architecture which was output\
 from `uname' when configure ran.])
  ;;

  # Hope to have a port to IRIX
  IRIX*)
  AC_DEFINE(QP_ARCH_IRIX, [],
   [The OS or architecture which was output\
 from `uname' when configure ran.])
  ;;


esac


##################################################
#     check for program pkg-config
##################################################


AC_CHECK_PROG(PKGCONFIG, pkg-config, [yes], [no])
if  test "$PKGCONFIG" = "no" ; then
  AC_MSG_ERROR([

  You need the program pkg-config in your PATH.
  See: http://www.freedesktop.org/software/pkgconfig/
  for more information.  Without this it's hard to say
  what else you need.

configure failed.
])
fi


#####################################################
#  Looking for packages using pkg-config
#####################################################

# will set later if we have a valid version of gtkmm
have_gtkmm=no

# will set later if we have a valid version of libsndfile
have_libsndfile=no


#########################################
#   check for package gtkmm
#########################################

gtkmm_need_version=2.4.5

echo -n "checking for gtkmm version >= $gtkmm_need_version ..."

GTKMM_PKG_NAME=gtkmm-2.4

# is gtkmm-2.4 there
if ! pkg-config --exists $GTKMM_PKG_NAME ; then
    AC_MSG_WARN([


  You need  gtkmm installed.  At least version $gtkmm_need_version
  The gtkmm home page is at http://www.gtkmm.org/

])

# is it at least version
elif ! pkg-config --atleast-version=$gtkmm_need_version $GTKMM_PKG_NAME ; then
   GTKMM_VERSION=$(pkg-config --modversion $GTKMM_PKG_NAME)
   AC_MSG_WARN([


  You have version $GTKMM_VERSION of gtkmm.
  You need at least version $gtkmm_need_version of gtkmm.
  The gtkmm home page is at http://www.gtkmm.org/

])
else
  GTKMM_VERSION=$(pkg-config --modversion $GTKMM_PKG_NAME)
  have_gtkmm=yes
  echo " yes"
fi


#########################################
#   check for package lib sndfile
#########################################

sndfile_need_version=1.0.5
have_libsndfile=no

echo -n "checking for lib sndfile version >= $sndfile_need_version ..."

# is it there
if ! pkg-config --exists sndfile ; then
    AC_MSG_NOTICE([


  The libsndfile library was not found.
  Quickplot can't read sound files without libsndfile.
  The lib sndfile home page is at http://www.mega-nerd.com/libsndfile/

])

# is it at least version
elif ! pkg-config --atleast-version=$sndfile_need_version sndfile ; then
   AC_MSG_WARN([


  You have version $SNDFILE_VERSION of libsndfile.
  You need at least version $sndfile_need_version of libsndfile.
  Quickplot can't read sound files without lib sndfile.
  The lib sndfile home page is at http://www.mega-nerd.com/libsndfile/

])
else
  have_libsndfile=yes
  echo " yes"
fi


#########################################
#  fail if gtkmm package is not found
#########################################

case "$have_gtkmm" in
    no)
  AC_MSG_ERROR([see WARNING(s) above: a usable\
 version of gtkmm was not found: configure failed.
])
    ;;
esac



#########################################
#  get library gtkmm compiler options
#########################################

GTKMM_CXXFLAGS=$(pkg-config --cflags $GTKMM_PKG_NAME)
AC_SUBST(GTKMM_CXXFLAGS)
GTKMM_LIBS=$(pkg-config --libs $GTKMM_PKG_NAME)
AC_SUBST(GTKMM_LIBS)



#########################################
#  get library sndfile compiler options
#########################################


AC_ARG_WITH([libsndfile],
      AS_HELP_STRING([--with-libsndfile],
      [link with the libsndfile library, http://www.mega-nerd.com/libsndfile/\
 (default is Yes if it is found, No if not)]),
      [use_libsndfile=$withval], [use_libsndfile=$have_libsndfile])

case "$use_libsndfile" in
    y* | Y* )
    use_libsndfile=yes
    ;;
    * )
    use_libsndfile=no
    ;;
esac


USE_LIBSNDFILE=0
SNDFILE_CFLAGS=""
SNDFILE_LIBS=""
SNDFILE_VERSION="none"

if test x${use_libsndfile} = xyes && test x${have_libsndfile} = xno; then
   AC_MSG_ERROR([
  A usable version of the libsndfile package was not found: configure failed.

   ])
fi

if test x${use_libsndfile} = xyes ; then
   # Using libsndfile with quickplot
   USE_LIBSNDFILE=1
   SNDFILE_CFLAGS=$(pkg-config --cflags sndfile)
   SNDFILE_LIBS=$(pkg-config --libs sndfile)
   SNDFILE_VERSION=$(pkg-config --modversion sndfile)
fi

if test "$USE_LIBSNDFILE" == "1"
then
  AC_DEFINE_UNQUOTED(USE_LIBSNDFILE, [],
   [If we are linking with the libsndfile\
 package USE_LIBSNDFILE will be defined.])
fi

AC_DEFINE_UNQUOTED(SNDFILE_VERSION, "$SNDFILE_VERSION", 
         [The version of libsndfile we're linking Quickplot with.])
AC_SUBST(SNDFILE_CFLAGS)
AC_SUBST(SNDFILE_LIBS)


########################################
# --with-html-dir=PATH
########################################

htmldir=${datadir}/doc/quickplot/html

AC_ARG_WITH([html-dir],
      AS_HELP_STRING([--with-html-dir=DIR],
      [directory to install html documents. \
The default is ${datadir}/doc/quickplot/html]),
      [htmldir=$withval], [htmldir=${datadir}/doc/quickplot/html])

AC_SUBST(htmldir)


#########################################
# extra variables
#########################################


# put RELEASE_DATE in Makefile
AC_SUBST(RELEASE_DATE)
# put RELEASE_DATE in config.h
AC_DEFINE_UNQUOTED(RELEASE_DATE, "$RELEASE_DATE",
 [The date on which the Quickplot (quickplot)\
 release was generated.])

# put HOMEPAGE in Makefile
AC_SUBST(HOMEPAGE)
# put HOMEPAGE in config.h
AC_DEFINE_UNQUOTED(HOMEPAGE, "$HOMEPAGE",
 [The Quickplot (quickplot) home-page.])

CONFIGURE_DATE="`date`"
# put CONFIGURE_DATE in the Makefiles
AC_SUBST(CONFIGURE_DATE)
# put CONFIGURE_DATE in config.h
AC_DEFINE_UNQUOTED(CONFIGURE_DATE, "$CONFIGURE_DATE",
 [The date on which the Quickplot (quickplot) was\
 configured by running the GNU Autotools autoconf configure script.])

AC_SUBST(TARNAME)


if test "${MAKE}" = "" ; then
  MAKE_ifset="Is not set, but that is likely OK."
else
  MAKE_ifset="${MAKE}"
fi

####################################################################


AC_CONFIG_FILES([
Makefile
VERSION
])

AC_OUTPUT

# you can add spew here

AC_MSG_RESULT([
  ------------------------------------------------------
  done configuring: $PACKAGE_NAME  version $VERSION
  ------------------------------------------------------

     Linking with lib gtkmm version:      $GTKMM_VERSION
     Linking with libsndfile?:            $use_libsndfile
     Linking with libsndfile version:     $SNDFILE_VERSION
     C++ Compiler (CXX):                  $CXX
     C++ Compiler Flags (CXXFLAGS):       $CXXFLAGS
     C++ Linker Flags (LDFLAGS):          $LDFLAGS
     Linker (LD):                         $LD
     C/C++ preprocessor flags (CPPFLAGS): $CPPFLAGS
     Installer (INSTALL):                 $INSTALL
     Make (MAKE):                         $MAKE_ifset
     htmldir                              $htmldir
     Installation prefix (prefix):        $prefix

])
