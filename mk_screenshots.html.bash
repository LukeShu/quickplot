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


sed\
 -e '/@STYLE_CSS@/ {
	r style.css
	d 
     }'\
 -e '/@THUMBS_HTM@/ {
	r thumbs.htm
	d
     }'\
 -e '/@FOOTER_HTM@/ {
	r footer.htm
	d
     }' | sed\
 -e 's!@Top@!<a class=nav href="index.html">\&nbsp;Top\&nbsp;</a>!'\
 -e 's!@About@!<a class=nav href="about.html">\&nbsp;About\&nbsp;</a>!'\
 -e 's!@Help@!<a class=nav href="help.html">\&nbsp;Help\&nbsp;</a>!'\

