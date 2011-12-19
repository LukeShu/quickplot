#!/bin/sh

ret="`(svnversion) 2>/dev/null`"
err="$?"

if test "X$err" != "X0" ; then
  ret=exported
elif test -z "$ret" ; then
  ret=unknown
fi

if test "X$ret" = "Xexported" ; then
  if test -f REPO_VERSION ; then
    out="`cat REPO_VERSION`"
    echo "$out"
  else
    echo "unknown"
  fi
else
  echo $ret
fi
