# This is a Gentoo GNU/Linux ebuild script file for package quickplot.
#run%  ACCEPT_KEYWORDS="~x86" emerge quickplot

DESCRIPTION="A fast interactive 2-D plotter"
HOMEPAGE="http://quickplot.sourceforge.net/"
SRC_URI="mirror://sourceforge/${PN}/${P}.tar.bz2"

LICENSE="GPL-2"
SLOT="0"
KEYWORDS="~x86"
IUSE=""
DEPEND="\
>=dev-cpp/gtkmm-2.4.5 \
>=dev-util/pkgconfig-0.15 \
>=media-libs/libsndfile-1.0.5"

src_unpack() {

	unpack ${P}.tar.bz2 || die
}

src_compile() {

	econf || die
	emake || die
}

src_install() {

	einstall || die
}
