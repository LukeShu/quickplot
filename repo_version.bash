#!/bin/bash

if test -z "$2" ; then
  echo "Usage: $0 REPO_VERSION_FILE SRC_DIR"
  exit 1
fi

cd "$2" || exit 1

if cdup="x$(git rev-parse --show-cdup 2>/dev/null)" && [ "$cdup" = x ] ; then
  # Git

  mod=
  if git status -s | egrep -q '^.?M' ; then
    mod=M # the source has modifications
  fi

  ret="$(git log --oneline | wc -l)"

  echo "${ret}${mod}"
else
  # non-Git
  if [ -f "$1" ] ; then
    cat "$1"
  else
    echo "Unknown"
  fi
fi
