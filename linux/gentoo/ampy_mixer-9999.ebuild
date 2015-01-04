# Copyright 1999-2014 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: /var/cvsroot/gentoo-x86/app-admin/ansible/ansible-9999.ebuild,v 1.23 2014/12/02 08:23:05 pinkbyte Exp $

EAPI=5

inherit eutils libtool git-r3

DESCRIPTION="mixer software for the ampy receiver"
HOMEPAGE="http://github.com/rodan/ampy/"
SRC_URI=""
EGIT_REPO_URI="git://github.com/rodan/ampy.git"
EGIT_BRANCH="master"
S="${WORKDIR}/${P}/linux"

LICENSE="GPL-3"
SLOT="0"
KEYWORDS="x86 amd64"
IUSE=""

DEPEND="sys-libs/ncurses"
RDEPEND=""

#src_prepare() {
#	elibtoolize
#}

src_install() {
	dodir "${ROOT}/usr/bin"
	exeinto "${ROOT}/usr/bin"
	doexe src/ampy_mixer
}

