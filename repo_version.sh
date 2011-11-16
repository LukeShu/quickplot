#!/bin/sh

ret="`(svnversion) 2>/dev/null`"
err="$?"


if test "X$err" != "X0" || test "X$ret" = "Xexported" ; then
  if test -f REPO_VERSION ; then
    cat REPO_VERSION
  else
    echo "unknown"
  fi
else
  echo $ret
fi

