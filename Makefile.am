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

# The .xpm files are built but come in a tarball .xpm files are
# not checked into the repository.  Distributing the .xpm file in
# the tarball removes the ImageMagick 'convert' program dependency.

XPM_FILES =\
 imgCopyWindow.xpm\
 imgDeleteWindow.xpm\
 imgGrabCursor.xpm\
 imgHoldCursor.xpm\
 imgNewWindow.xpm\
 imgSaveImage.xpm\
 quickplot.xpm

EXTRA_DIST =\
 house.png\
 imgGrabCursor.png\
 imgHoldCursor.png\
 imgNewWindow.png\
 imgDeleteWindow.png\
 imgCopyWindow.png\
 imgSaveImage.png\
 quickplot.png\
 sample_plot.png\
 screenshot_small.png\
 favicon.ico\
 background_ft.png\
 background_h1.png\
 background_h2.png\
 style.css\
 about.txt\
 help.txt\
 bootstrap\
 quickplot.ebuild\
 Repo_clean\
 release_date.bash\
 repo_version.bash\
 ax_lib_readline.m4

BUILT_SOURCES =\
 paths.h\
 parse_args.h\
 app_op_declare.h\
 app_op_init.h

BUILT_SOURCES +=\
 $(XPM_FILES)

BUILT_SOURCES +=\
 about.txt\
 help.txt


man_MANS = quickplot.1 quickplot_shell.1

html_DATA =\
 about.html\
 help.html\
 house.png\
 index.html\
 quickplot.png\
 sample_plot.png\
 screenshot_small.png\
 background_ft.png\
 background_h1.png\
 background_h2.png\
 favicon.ico\
 about.txt\
 help.txt


DISTCLEANFILES =\
 about.html\
 app_op_declare.h\
 app_op_init.h\
 help.html\
 index.html\
 parse_args.h\
 paths.h\
 quickplot.1


if HAVE_LOCAL_SCREENSHOTS
html_DATA +=\
 screenshots.html

DISTCLEANFILES +=\
 screenshots.html
endif


noinst_PROGRAMS = mk_options
mk_options_SOURCES = mk_options.c


lib_LTLIBRARIES = libquickplot.la

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
 graph_detail.c\
 graph_draw.c\
 graph_grid.c\
 launch_browser.c\
 list.c\
 list.h\
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
 shell.c\
 shell_get_set_values.h\
 shell.h\
 source.c\
 source_double.c\
 shell_common.c\
 shell_common.h\
 shell_server_commands.c\
 spew.c\
 spew.h\
 term_color.c\
 term_color.h\
 utils.h\
 win.c\
 $(XPM_FILES)\
 zoom.h

libquickplot_la_SHORTNAME = lib
libquickplot_la_CFLAGS = $(gtk_3_CFLAGS) $(sndfile_CFLAGS)
libquickplot_la_LDFLAGS = -version-info $(LIB_VERSION)
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

# We look at a sndfile symbol when libquickplot runs
libquickplot_la_LIBADD = $(sndfile_LIBS)

if QP_DEBUG
libquickplot_la_SOURCES += debug_spew.c
else
EXTRA_DIST += debug_spew.c
endif



pkgconfigdir   = $(libdir)/pkgconfig
pkgconfig_DATA = quickplot.pc

quickplotinclude_HEADERS = quickplot.h
quickplotincludedir = $(includedir)



bin_PROGRAMS = quickplot quickplot_shell


quickplot_SOURCES =\
 config.h\
 qp_main.c\
 quickplot.h

quickplot_SHORTNAME = qp
quickplot_CFLAGS = $(gtk_3_CFLAGS) $(sndfile_CFLAGS)
quickplot_LDADD = libquickplot.la $(gtk_3_LIBS) $(sndfile_LIBS) -lX11 -ldl
#if USE_READLINE
quickplot_LDADD += $(readline_LIBS)
#endif


quickplot_shell_SOURCES =\
 list.c\
 debug.h\
 list.h\
 qp_shell.c\
 shell_common.c\
 shell_common.h\
 spew.c\
 spew.h\
 term_color.c\
 term_color.h

if QP_DEBUG
quickplot_shell_SOURCES += debug_spew.c
endif
quickplot_shell_SHORTNAME = sh
#if USE_READLINE
quickplot_shell_LDADD = $(readline_LIBS)
#endif



gzfile  = $(distdir).tar.gz
bz2file = $(distdir).tar.bz2
xzfile  = $(distdir).tar.xz


SUFFIXES = .html .bz2 .xz .gz \
.gz.md5 .gz.sha1 .gz.sha256 .gz.sha512 \
.bz2.md5 .bz2.sha1 .bz2.sha256 .bz2.sha512 \
.xz.md5 .xz.sha1 .xz.sha256 .xz.sha512

SUFFIXES += .png .xpm
SUFFIXES += .txt

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

quickplot.1: quickplot.1.head quickplot.1.tail mk_options
	echo ".\\\" Do not edit this file" > $@
	echo ".\\\" This file was generated using $(MAKE)" >> $@
	cat quickplot.1.head >> $@
	./mk_options -m >> $@
	cat quickplot.1.tail >> $@
	echo ".\\\" Do not edit this file" >> $@
	echo ".\\\" This file was generated using $(MAKE)" >> $@


index.html: index.html.fin
	sed $<\
 -e 's!@Top@!<span class=nav>\&nbsp;Top\&nbsp;</span>!'\
 -e 's!@About@!<a class=nav href="about.html">\&nbsp;About\&nbsp;</a>!'\
 -e 's!@Help@!<a class=nav href="help.html">\&nbsp;Help\&nbsp;</a>!'\
 > $@


about.html: about.html.fin
	sed $<\
 -e 's!@Top@!<a class=nav href="index.html">\&nbsp;Top\&nbsp;</a>!'\
 -e 's!@About@!<span class=nav>\&nbsp;About\&nbsp;</span>!'\
 -e 's!@Help@!<a class=nav href="help.html">\&nbsp;Help\&nbsp;</a>!'\
 > $@

help.html.tail: help.html.tail.fin
	sed $<\
 -e 's!@Top@!<a class=nav href="index.html">\&nbsp;Top\&nbsp;</a>!'\
 -e 's!@About@!<a class=nav href="about.html">\&nbsp;About\&nbsp;</a>!'\
 -e 's!@Help@!<span class=nav>\&nbsp;Help\&nbsp;</span>!'\
 > $@

help.html: help.html.head mk_options help.html.tail
	cat help.html.head > $@
	./mk_options -t >> $@
	cat help.html.tail >> $@

if HAVE_LOCAL_SCREENSHOTS

install-exec-hook:
	mkdir -p $(DESTDIR)$(htmldir)
	cp ScreenShot_* _ScreenShot_* $(DESTDIR)$(htmldir)

thumbs.html.part: mk_thumbs.htm.bash
	./mk_thumbs.htm.bash > $@

# ./config.status is part of ./configure; we do this here instead of
# adding screenshots.html.fin to AC_CONFIG_FILES to avoid
# screenshots.html.fin.in getting included in the dist tarball.
screenshots.html.fin: screenshots.html.fin.in style.css footer.html.part config.status
	./config.status --file=$@:$<

screenshots.html: screenshots.html.fin thumbs.html.part
	sed -e "/@THUMBS_HTM@/{ r thumbs.html.part$$(printf '\n ')d; }" $< > $@.tmp
	sed\
 -e 's!@Top@!<a class=nav href="index.html">\&nbsp;Top\&nbsp;</a>!'\
 -e 's!@About@!<a class=nav href="about.html">\&nbsp;About\&nbsp;</a>!'\
 -e 's!@Help@!<a class=nav href="help.html">\&nbsp;Help\&nbsp;</a>!'\
 $@.tmp > $@
	rm $@.tmp

screenshots:
	rm -f screenshots.html thumbs.html.part
	$(MAKE) screenshots.html

endif

help.html.head: style.css
help.html.tail.fin: footer.html.part
index.html.fin: style.css footer.html.part
about.html.fin: style.css footer.html.part



.png.xpm:
	$(CONVERT) $< xpm:$@.tmp
	sed -e 's/static.*\schar/static const char/g' -e 's/_xpm\[\] = /[] = /g' $@.tmp > $@
	rm $@.tmp


.html.txt:
	$(LYNX) -dump $< > $@.tmp
	dir=`pwd` && sed -e "s!file://$$dir/!!g" $@.tmp > $@
	rm $@.tmp


# Add the repository revision number to the dist.
# Fix any dist files that have RELEASE_DATE in them, like help.txt
# Remove generated dist files that auto-tools is stupid about.
# Remove the out-of-date hash files.
dist-hook:
	if test -f RELEASE_DATE ; then mv RELEASE_DATE RELEASE_DATE.save ; fi
	if test -f REPO_VERSION ; then mv REPO_VERSION REPO_VERSION.save ; fi
	${top_srcdir}/repo_version.bash ${top_srcdir}/REPO_VERSION ${top_srcdir} > REPO_VERSION.tmp
	mv REPO_VERSION.tmp REPO_VERSION
	date -u +"%d %b %G %H:%M:%S UTC" > RELEASE_DATE
	echo $(VERSION) > last_release
	cat RELEASE_DATE REPO_VERSION >> last_release
	rm -f about.txt about.html about.html.fin
	$(MAKE) about.txt
	mv REPO_VERSION RELEASE_DATE about.txt $(distdir)
	rm -f about.txt about.html about.html.fin
	$(MAKE) about.txt
	if test -f REPO_VERSION.save ; then mv REPO_VERSION.save REPO_VERSION ; fi
	if test -f RELEASE_DATE.save ; then mv RELEASE_DATE.save RELEASE_DATE ; fi
	cd $(distdir) && rm -f $(DISTCLEANFILES) config.h
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

.DELETE_ON_ERROR:
