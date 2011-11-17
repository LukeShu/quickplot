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
####################################################################


EXTRA_DIST =\
 imgGrabCursor.png\
 imgGrabCursor.xpm\
 imgHoldCursor.png\
 imgHoldCursor.xpm\
 imgNewWindow.png\
 imgNewWindow.xpm\
 imgDeleteWindow.png\
 imgDeleteWindow.xpm\
 imgCopyWindow.png\
 imgCopyWindow.xpm\
 imgSaveImage.png\
 imgSaveImage.xpm\
 quickplot_icon.png\
 quickplot_icon.xpm\
 screenshot1.png\
 screenshot1_small.png\
 screenshot2.png\
 screenshot3.png\
 background.png\
 favicon.ico\
 style.css\
 footer.htm.in\
 about.htm.in\
 help_head.htm.in\
 help_tail.htm.in\
 index.html.in\
 about.txt\
 help.txt\
 bootstrap\
 Repo_clean\
 README.devel\
 TODO\
 _header.c\
 repo_version.sh

BUILT_SOURCES =\
 imgGrabCursor.xpm\
 imgHoldCursor.xpm\
 imgNewWindow.xpm\
 imgDeleteWindow.xpm\
 imgCopyWindow.xpm\
 imgSaveImage.xpm\
 quickplot_icon.xpm\
 paths.h\
 parse_args.h\
 app_op_declare.h\
 app_op_init.h\
 help.html

# The .xpm files are built but come in a tarball
# .xpm files are not checked into the repository

html_DATA =\
 about.html\
 help.html\
 index.html\
 quickplot_icon.png\
 screenshot1.png\
 screenshot1_small.png\
 screenshot2.png\
 screenshot3.png\
 background.png\
 favicon.ico\
 about.txt\
 help.txt


DISTCLEANFILES =\
 about.html\
 app_op_declare.h\
 app_op_init.h\
 help.html\
 parse_args.h\
 paths.h



noinst_PROGRAMS = mk_options
mk_options_SOURCES = mk_options.c


libquickplot_la_SOURCES =\
 app_op_declare.h\
 app_op_init.h\
 callbacks.c\
 callbacks.h\
 channel.c\
 channel_double.c\
 channel_double.h\
 channel.h\
 color_gen.c\
 color_gen.h\
 config.h\
 debug.h\
 get_opt.c\
 get_opt.h\
 graph.c\
 graph_draw.c\
 graph_grid.c\
 imgCopyWindow.xpm\
 imgDeleteWindow.xpm\
 imgGrabCursor.xpm\
 imgHoldCursor.xpm\
 imgNewWindow.xpm\
 imgSaveImage.xpm\
 launch_browser.c\
 list.c\
 list.h\
 main_window.c\
 parse_1st_pass_funcs.h\
 parse_2nd_pass_funcs.h\
 parse_args.h\
 parse_args_utils.h\
 parse_argument_options.c\
 paths.h\
 plot.c\
 plot.h\
 process_gtk_options.c\
 qp.c\
 qp.h\
 quickplot.h\
 quickplot_icon.xpm\
 shell.c\
 source.c\
 source_double.c\
 spew.c\
 spew.h\
 term_color.c\
 term_color.h\
 zoom.h

libquickplot_la_SHORTNAME = lib

libquickplot_la_CFLAGS = $(gtk_3_CFLAGS) $(sndfile_CFLAGS)

libquickplot_la_LDFLAGS = -version-info $(LIB_VERSION) $(gtk_3_LIBS) $(sndfile_LIBS)

# For making libquickplot.so executable if we are using GCC
# and glibc which we tested in configure by compiling with the
# gnu/lib-names.h file.
if HAVE_GNU_LIB_NAMES_H
libquickplot_la_SOURCES += exec_lib.c
libquickplot_la_LDFLAGS += -shared -Wl,-e -Wl,qp_exec_lib
libquickplot_la_CFLAGS  += -fPIC
else
EXTRA_DIST += exec_lib.c
endif

if QP_DEBUG
libquickplot_la_SOURCES += debug_spew.c
else
EXTRA_DIST += debug_spew.c
endif


lib_LTLIBRARIES = libquickplot.la

pkgconfigdir   = $(libdir)/pkgconfig
pkgconfig_DATA = quickplot.pc

quickplotinclude_HEADERS = quickplot.h
quickplotincludedir = $(includedir)



bin_PROGRAMS = quickplot

quickplot_SOURCES =\
 config.h\
 qp_main.c\
 quickplot.h

quickplot_SHORTNAME = qp

# LDADD to add linking to libraries not found by configure
quickplot_LDADD = libquickplot.la 
quickplot_CFLAGS = $(gtk_3_CFLAGS) $(sndfile_CFLAGS)
quickplot_LDFLAGS = $(gtk_3_LIBS) $(sndfile_LIBS)


gzfile  = $(distdir).tar.gz
bz2file = $(distdir).tar.bz2
xzfile  = $(distdir).tar.xz


SUFFIXES = .html .bz2 .xz .gz \
.gz.md5 .gz.sha1 .gz.sha256 .gz.sha512 \
.bz2.md5 .bz2.sha1 .bz2.sha256 .bz2.sha512 \
.xz.md5 .xz.sha1 .xz.sha256 .xz.sha512

if HAVE_CONVERT
SUFFIXES += .png .xpm
endif

if HAVE_LYNX
SUFFIXES += .txt
endif


.PHONY: hash md5 sha1 sha256 sha512

# TODO: There should be a better way to find installed
# from the running quickplot program.
paths.h:
	echo "/* This file was generated using $(MAKE) */" > $@
	echo >> $@
	echo "#define HTMLDIR \"$(htmldir)\"" >> $@
	echo "#define DOCDIR \"$(docdir)\"" >> $@

parse_args.h: mk_options
	./mk_options -a > $@
app_op_declare.h: mk_options
	./mk_options -I > $@
app_op_init.h: mk_options
	./mk_options -i > $@

help.html: help_head.htm mk_options help_tail.htm
	cat help_head.htm > $@
	./mk_options -t >> $@
	cat help_tail.htm >> $@


help_head.htm: style.css
help_tail.htm: footer.htm
index.html: style.css  footer.htm
about.htm: style.css footer.htm


# This is just so we have at least one installed file,
# about.html, that will tell us what the package source
# is.  This cannot be done in configure because we
# want to be able to change repo_version (by checking
# in file and stuff) without having to re-run configure.
about.html: about.htm
	reves="`./repo_version.sh`" ; sed -e "s/@REPO_VERSION@/$$reves/" $< > $@

if HAVE_CONVERT
.png.xpm:
	convert $< $@
	mv $@ $@.in
	sed -e 's/static.*\schar/static const char/g' $@.in > $@
	rm $@.in
endif


if HAVE_LYNX
.html.txt:
	echo > $@
	dir=`pwd` && lynx -dump $< | \
sed -e "s!//localhost$$dir!//$(htmldir)!g" >> $@
endif




# If we remake the dist we must remove the out-of-date hash files.
dist-hook:
	./repo_version.sh > $(distdir)/REPO_VERSION
	rm -f $(gzfile).md5 $(gzfile).sha* $(bz2file).md5 $(bz2file).sha* $(xzfile).md5 $(xzfile).sha*


hash: md5 sha1 sha256 sha512
md5:    $(gzfile).md5    $(bz2file).md5    $(xzfile).md5
sha1:   $(gzfile).sha1   $(bz2file).sha1   $(xzfile).sha1
sha256: $(gzfile).sha256 $(bz2file).sha256 $(xzfile).sha256
sha512: $(gzfile).sha512 $(bz2file).sha512 $(xzfile).sha512

.gz.gz.md5: 
	md5sum $< > $@
.gz.gz.sha1:
	sha1sum $< > $@
.gz.gz.sha256:
	sha256sum $< > $@
.gz.gz.sha512:
	sha512sum $< > $@

.bz2.bz2.md5: 
	md5sum $< > $@
.bz2.bz2.sha1:
	sha1sum $< > $@
.bz2.bz2.sha256:
	sha256sum $< > $@
.bz2.bz2.sha512:
	sha512sum $< > $@

.xz.xz.md5:
	md5sum $< > $@
.xz.xz.sha1:
	sha1sum $< > $@
.xz.xz.sha256:
	sha256sum $< > $@
.xz.xz.sha512:
	sha512sum $< > $@

$(gzfile):
	$(MAKE) dist-gzip

$(bz2file):
	$(MAKE) dist-bzip2

$(xzfile):
	$(MAKE) dist-xz

