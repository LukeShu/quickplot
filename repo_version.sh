#!/bin/sh

if test -z "$2" ; then
  echo "Usage: $0 REPO_VERSION_FILE SRC_DIR"
  exit 1
fi

ret="`(svnversion) 2>/dev/null`"
err="$?"

if test "$err" != "0" ; then
  if test -d "$2/.svn" ; then
    echo "unknown"
  else
    cat "$1"
  fi
elif test "$ret" = "Unversioned directory" ; then
  cat "$1"
else
  echo "$ret"
fi
