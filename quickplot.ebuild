# This is a Gentoo GNU/Linux ebuild script file.  See
# http://www.gentoo.org/doc/en/gentoo-howto.xml.  If your not using a
# Gentoo system and/or Gentoo portage don't worry about this file.

# If you're testing a release:
#run%  ACCEPT_KEYWORDS="~x86" emerge quickplot
#or
#run% MAKE_FROM_CVS_SOURCE=yes ACCEPT_KEYWORDS="~x86" emerge quickplot

DESCRIPTION="A fast interactive 2-D plotter"
HOMEPAGE="http://quickplot.sourceforge.net/"
SRC_URI="mirror://sourceforge/${PN}/${P}.tar.bz2"

LICENSE="GPL"
SLOT="0"
KEYWORDS="x86"
IUSE=""
DEPEND="\
>=dev-cpp/gtkmm-2.2.8 \
>=dev-util/pkgconfig-0.15 \
>=media-libs/libsndfile-1.0.5"

# if this was installed without the autoconf configure script then we
# need the GNU Autotools: autoconf, automake and libtool.  Also
# imagemagick was used to make xpm image file(s) from png files.

if [ "${MAKE_FROM_CVS_SOURCE}" = "yes" ] ; then
	DEPEND="${DEPEND} >=sys-devel/autoconf-2.58 \
       >=sys-devel/automake-1.7 \
       >=sys-devel/libtool-1.4.3 \
       >=media-gfx/imagemagick-5.5.7.15"
fi


src_unpack() {

	unpack ${P}.tar.bz2 || die
}

src_compile() {

	if [ ! -e ./configure ] ; then
		# ./bootstrap calls aclocal, autoconf, automake and libtoolize.
        if ! ./bootstrap ; then
			if [ "${MAKE_FROM_CVS_SOURCE}" != "yes" ] ; then
				die "./bootstrap failed
  Try setting environment variable MAKE_FROM_CVS_SOURCE=yes
  and then run this again.  This may add newer GNU Autotools and imagemagick."
			else
				die  "./bootstrap failed"
			fi
		fi
	fi

	econf || die
	emake || die
}

src_install() {

	einstall || die
}

