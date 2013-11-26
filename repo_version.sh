#!/bin/sh

if test -z "$1" ; then
  echo "bad script $0 usage"
  exit 1
fi

ret="`(svnversion) 2>/dev/null`"
err="$?"

if test "$err" != "0" ; then
  echo "unknown"
elif test "$ret" = "Unversioned directory" ; then
  echo "unknown"
else
  echo "$ret"
fi
