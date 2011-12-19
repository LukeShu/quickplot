#!/bin/sh

if test -z "$1" ; then
  echo "bad script $0 usage"
  exit 1
fi

if test -f "$1" ; then
  cat "$1"
else
  echo "not released yet"
fi
