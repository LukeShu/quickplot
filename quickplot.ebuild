# Copyright 1999-2005 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header$

DESCRIPTION="A fast interactive 2D plotter."
SRC_URI="mirror://sourceforge/${PN}/${P}.tar.bz2"
HOMEPAGE="http://quickplot.sourceforge.net/"

IUSE="sndfile"
SLOT="0"
LICENSE="GPL-2"
KEYWORDS="~x86 ~amd64"

RDEPEND=">=dev-cpp/gtkmm-2.4.5 >=media-libs/libsndfile-1.0.5"

DEPEND="${RDEPEND} >=dev-util/pkgconfig-0.15"

src_compile() {
        econf || die "econf step failed."
        emake htmldir=/usr/share/doc/${PF}/html || die "emake step failed."
}

src_install () {
        make install DESTDIR=${D} htmldir=/usr/share/doc/${PF}/html \
                || die "make install step failed."
        dodoc AUTHORS ChangeLog README README.devel TODO
        mv ${D}/usr/share/pixmaps/quickplot_icon.png \
                ${D}/usr/share/pixmaps/quickplot.png
}
