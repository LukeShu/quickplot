
homepagefiles =\
 about_browser16x16.png\
 closeFrame16x16.png\
 copyFrame16x16.png\
 newFrame16x16.png\
 quickplot_icon.png\
 saveImage16x16.png\
 screenshot1.png\
 screenshot1_small.png\
 screenshot2.png\
 screenshot3.png\
 about.html\
 help.html\
 index.html\
 ChangeLog\
 README\
 README.devel\
 TODO\
 quickplot.ebuild


htmldir = $(datadir)/doc/quickplot/html

html_DATA =\
 help.html\
 about.html\
 index.html\
 quickplot_icon.png\
 screenshot1_small.png\
 screenshot1.png\
 screenshot2.png\
 screenshot3.png\
 ChangeLog\
 favicon.ico\
 COPYING

bin_PROGRAMS = quickplot

# after building: quickplot_SOURCES = `echo *.cpp *.h'
quickplot_SOURCES =\
 App.cpp App_parseArgs.cpp ButtonBar.cpp ColorGen.cpp Field.cpp FieldReader.cpp File.cpp FileList.cpp FileReader.cpp File_readASCIIFile.cpp File_readSndDouble.cpp File_readSndFile.cpp File_readSndFloat.cpp File_readSndInt.cpp File_readSndShort.cpp Globel.cpp Graph.cpp GraphConfig.cpp Graph_drawAutoGrid.cpp LinearField.cpp MainMenuBar.cpp MainWindow.cpp Plot.cpp PlotConfig.cpp PlotList.cpp PlotLister.cpp PlotSelector.cpp Plot_cullDrawLine.cpp Source.cpp StatusBar.cpp ValueSlider.cpp about_html.cpp errorStr.cpp getOpt.cpp help_html.cpp launchBrowser.cpp main.cpp parseGeometry.cpp usage.cpp App.h ArrayField.h ButtonBar.h ColorGen.h Field.h FieldReader.h File.h FileList.h FileReader.h Globel.h Graph.h GraphConfig.h LinearField.h ListField.h LogField.h MainMenuBar.h MainWindow.h Plot.h PlotConfig.h PlotList.h PlotLister.h PlotSelector.h Source.h StatusBar.h ValueSlider.h errorStr.h mingw_config.h value_t.h config.h


BUILT_SOURCES =\
 quickplot_icon.xpm\
 about_browser16x16.xpm\
 newFrame16x16.xpm\
 closeFrame16x16.xpm\
 copyFrame16x16.xpm\
 saveImage16x16.xpm\
 File_readSndInt.cpp\
 File_readSndFloat.cpp\
 File_readSndDouble.cpp\
 help_html.cpp\
 about_html.cpp


quickplot_CXXFLAGS = -Wall $(GTKMM_CFLAGS) $(SNDFILE_CFLAGS) -DHTMLDIR=\"$(htmldir)\"

#Why won't this work?
#launchBrowser_CXXFLAGS = -DHTMLDIR=\"$(htmldir)\"

quickplot_LDFLAGS = -Wall $(GTKMM_LIBS) $(SNDFILE_LIBS)
# -lefence

# require automake 1.7
AUTOMAKE_OPTIONS = 1.7


# Quickplot Icon
icondir      = $(datadir)/pixmaps
icon_DATA    = quickplot_icon.png


# We include everything that you could need for development.  We hate
# it when packages do not include all the needed files for
# development.


EXTRA_DIST =\
 bootstrap\
 cvsclean\
 html2CString\
 install_portage\
 TODO\
 README.devel\
 quickplot.ebuild\
 help.html.in\
 about.html.in\
 index.html.in\
 quickplot_icon.xpm\
 about_browser16x16.xpm\
 newFrame16x16.xpm\
 closeFrame16x16.xpm\
 copyFrame16x16.xpm\
 saveImage16x16.xpm\
 about_browser16x16.png\
 newFrame16x16.png\
 closeFrame16x16.png\
 copyFrame16x16.png\
 saveImage16x16.png\
 quickplot.dev\
 mingw.mak\
 mingw_config.h\
 $(html_DATA)


SUFFIXES = .png .xpm .html .html.in _html.cpp 

.png.xpm:
	convert $< $@

.html_html.cpp:
	cat $< | ./html2CString $@


.html.in.html:
	@echo "making $@ from $<"
	@echo "<html>" > $@
	@echo "<!-- $@  Generated from $(MAKE) and source $< !-->" >> $@
	@echo >> $@
	cat $< | sed \
          -e "s/%PACKAGE%/$(PACKAGE)/g" \
          -e "s/%TARNAME%/$(TARNAME)/g" \
          -e "s^%HOMEPAGE%^$(HOMEPAGE)^g" \
          -e "s/%VERSION%/$(VERSION)/g" \
          -e "s^%RELEASE_DATE%^$(RELEASE_DATE)^g" \
          >> $@
	@echo "<center><hr></hr>" >> $@
	@if [ $@ != index.html ]; then\
          echo "<a href=\"index.html\">Top</a><hr></hr>" >> $@ ;\
          fi
	@echo "<a href=\"$(HOMEPAGE)\">Quickplot</a>" >> $@
	@echo "&nbsp;&nbsp; version ${VERSION} &nbsp;&nbsp; " >> $@
	@echo "release date: $(RELEASE_DATE)" >> $@
	@echo "</center>" >> $@
	@echo "</body>" >> $@
	@echo "</html>" >> $@
 

.PHONY: homepage



File_readSndInt.cpp: File_readSndShort.cpp
	rm -f $@
	echo "// $@  Generated from $(MAKE) and $<" >> $@
	echo "#define TYPE_INT" >> $@
	cat $< >> $@

File_readSndFloat.cpp: File_readSndShort.cpp
	rm -f $@
	echo "// $@  Generated from $(MAKE) and $<" >> $@
	echo "#define TYPE_FLOAT" >> $@
	cat $< >> $@

File_readSndDouble.cpp: File_readSndShort.cpp
	rm -f $@
	echo "// $@  Generated from $(MAKE) and $<" >> $@
	echo "#define TYPE_DOUBLE" >> $@
	cat $< >> $@

homepage: $(homepagefiles) dist
	tar -cjf quickplot_homepage-${VERSION}.tar.bz2 $(homepagefiles)
	md5sum quickplot-${VERSION}.tar.bz2 > quickplot-${VERSION}.tar.bz2.md5
	md5sum quickplot-${VERSION}.tar.gz > quickplot-${VERSION}.tar.gz.md5


mingw_config.h: config.h
	cp $^ $@
