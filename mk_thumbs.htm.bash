#! /bin/bash

# Quickplot - an interactive 2D plotter
#
# Copyright (C) 1998-2011  Lance Arsenault

# This file is part of Quickplot.
#
# Quickplot is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 3 of the License,
# or (at your option) any later version.
#
# Quickplot is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Quickplot.  If not, see <http://www.gnu.org/licenses/>.
#

#---------------------------------------------------------------------
#                         WHAT'S THIS FOR

# This makes the image thumb nail links for the screenshot.html page.
# This makes the thumb nail image files too.

#---------------------------------------------------------------------

d=$(echo "$0"|sed 's/[^/]*$//')
[ -z "$d" ] && d='.'
if [ "$(cd "$d" 2>/dev/null && echo "${PWD}")" != "${PWD}" ]
then
    echo "This script ($0) must be executed directly\
 from the top $0 source directory."
    exit 1
fi
 
progs="svn svnversion convert autoconf automake sed grep awk bash diff"
inst=
for prog in $progs ; do
    if ! which $prog &> /dev/null ; then
       echo " $prog is not installed in your PATH"
       inst="$inst $prog"
    fi
done

if [ -n "$inst" ] ; then
    echo
    echo " You need to have the following installed to run this:"
    echo
    echo "    $inst"
    echo
    echo " or else you will not be able to recreate the files that"
    echo " running this removes."
    exit 1
fi


function Get_Prefixes()
{
  local Pre=ScreenShot_
  local count=0

  while [ -n "$1" ] ; do
    for i in ${Pre}*${1} ; do
      prefix[$count]="${i%${1}}"
      for i in ${prefix[$count]}* ; do
        if [ -n "${files[$count]}" ] ; then
          files[$count]="${files[$count]} $i"
        else
          files[$count]="$i"
        fi
      done
      let count=$count+1
    done
    shift 1
  done
  prefix[$count]=
}

Get_Prefixes ".png"

function Debug_print()
{
  i=0
  while [ -n "${prefix[$i]}" ] ; do
    echo "prefix: ${prefix[$i]} files=\"${files[$i]}\""
    let i=$i+1
  done
}

#Debug_print
#exit

echo "<!-- This part of this file was generated with $0 -->"

i=0
while [ -n "${files[$i]}" ] ; do
  for f in ${files[$i]} ; do

    case "$f" in

      *.png|*.jpg)
        thumb="_${prefix[$i]}_thumb.png"
        html="${prefix[$i]}.html"
        if [ ! -f $html ] ; then
          echo "<a href=\"$f\"><img alt=\"image\" src=\"$thumb\"></a> "
        else
          echo "<a href=\"$html\"><img alt=\"image\" src=\"$thumb\"></a> "
        fi
        set -x
        convert $f -resize x100 $thumb
        set +x
        ;;
   
      *)
        ;;
    esac
    
  done
  let i=$i+1
done

echo "<!-- The above part of this file was generated with $0 -->"


