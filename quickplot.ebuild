# Copyright 1999-2005 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header$

DESCRIPTION="A fast interactive 2D plotter."
SRC_URI="mirror://sourceforge/${PN}/${P}.tar.bz2"
HOMEPAGE="http://quickplot.sourceforge.net/"

IUSE=""
SLOT="0"
LICENSE="GPL-2"
KEYWORDS="~x86 ~amd64"

DEPEND=">=dev-util/pkgconfig-0.15
        >=dev-cpp/gtkmm-2.4.5
        >=media-libs/libsndfile-1.0.5"

src_compile() {
        econf --with-html-dir=/usr/share/doc/${PF} || die "econf step failed."
        emake || die "emake step failed."
}

src_install () {
        make install DESTDIR=${D} || die "make install step failed."
        # Remove the licence as it is specified in LICENCE
        rm ${D}/usr/share/doc/${PN}/COPYING
        # Move the docs to the correct location
        mv ${D}/usr/share/doc/${PN} ${D}/usr/share/doc/${PF}
}
