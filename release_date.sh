#!/bin/sh

if test -f RELEASE_DATE ; then
  cat RELEASE_DATE
else
  echo "not released yet"
fi

