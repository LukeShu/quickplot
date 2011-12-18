#

EAPI="1"
inherit eutils

DESCRIPTION="A fast interactive 2D plotter."
SRC_URI="mirror://sourceforge/${PN}/${P}.tar.bz2"
HOMEPAGE="http://quickplot.sourceforge.net/"

SLOT="0"
LICENSE="GPL-3"
KEYWORDS="amd64 ~ppc ~x86"

RDEPEND="
	>=x11-libs/gtk+-3:3
	>=media-libs/libsndfile-1.0"
DEPEND="${RDEPEND}
	dev-util/pkgconfig"


src_install () {
	emake
		DESTDIR="${D}"\
		htmldir=/usr/share/doc/${PF}/html\
		install || die "emake install failed."
	dodoc AUTHORS README ChangeLog
	newicon quickplot.png ${PN}.png
	make_desktop_entry 'quickplot --no-pipe' Quickplot quickplot Graphics
	mv "${D}"/usr/share/applications/quickplot\ --no-pipe.desktop \
		"${D}"/usr/share/applications/quickplot.desktop
}
