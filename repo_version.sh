#!/bin/sh

if test -z "$1" ; then
  echo "bad script $0 usage"
  exit 1
fi

ret="`(svnversion) 2>/dev/null`"
err="$?"

if test "X$err" != "X0" ; then
  ret=exported
elif test -z "$ret" ; then
  ret=unknown
fi

if test "X$ret" = "Xexported" ; then
  if test -f "$1" ; then
    out="`cat $1`"
    echo "$out"
  else
    echo "unknown"
  fi
else
  echo $ret
fi
