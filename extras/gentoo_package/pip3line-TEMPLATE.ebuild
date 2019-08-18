# Copyright 1999-2018 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=6

PYTHON_COMPAT=( python{2_7,3_{4,5,6,7}} )

inherit cmake-utils python-r1

DESCRIPTION="Raw bytes manipulation, transformations (decoding and more) and interception"
HOMEPAGE="https://github.com/metrodango/pip3line"

if [[ ${PV} == 9999* ]] ; then
	inherit git-r3
	EGIT_REPO_URI="https://github.com/metrodango/pip3line.git"
	EGIT_BRANCH="master"
else
	DISTORMV="3.4.1"
	SRC_URI="https://github.com/metrodango/pip3line/archive/v${PV}.tar.gz  -> ${P}.tar.gz
			https://github.com/gdabah/distorm/archive/v${DISTORMV}.tar.gz -> distorm3-${DISTORMV}.tar.gz"
	KEYWORDS="~amd64 ~x86"
fi

LICENSE="GPL-3"
SLOT="0"

# A few comments
# the ssl flag is just there to enable the plugin for low level crypto algorithms. 
# It has nothing to do with the SSL/TLS protocol itself.

IUSE="distorm python qscintilla ssl"

RDEPEND="
	${PYTHON_DEPS}
	dev-qt/qtconcurrent:5
	dev-qt/qtcore:5
	dev-qt/qtgui:5
	dev-qt/qtnetwork:5
	dev-qt/qtsvg:5
	dev-qt/qtwidgets:5
	dev-qt/qtxmlpatterns:5
	qscintilla? ( x11-libs/qscintilla )
	ssl? ( dev-libs/openssl:0= )"

DEPEND="${RDEPEND}
	distorm? ( dev-vcs/git )"

src_unpack() {
	if [ "${A}" != "" ]; then
		unpack ${A}
		if [ "${DISTORMV}" != "" ]; then # we are in a versioned ebuild, we need to link the distorm src dir inside the pip3line one
		   
			cp -ar ${WORKDIR}/distorm-${DISTORMV}/* ${WORKDIR}/${P}/ext/distorm
		fi
	fi
}

src_configure() {

	local mycmakeargs=(
		-DBASIC=yes
		-DWITH_DISTORM=$(usex distorm)
		-DWITH_OPENSSL=$(usex ssl)
		-DWITH_SCINTILLA=$(usex qscintilla)
	)

	# distorm is statically linked, due to insufficiencies
	# in the current distorm64 package 

	if use distorm; then
		mycmakeargs+=(-DWITH_DISTORM_LINK_STATICALLY=ON)
	fi

	if use python; then
		local targets=( ${PYTHON_TARGETS//[_]/.} )
		for target in ${targets[@]}; do
			if [[ "${target}" == *3.* ]]; then
				einfo "Selected ${target}"
				mycmakeargs+=(-DWITH_PYTHON3=$(usex python)
					-DPYTHON3_INCLUDE_DIRS=/usr/include/${target}m
					-DPYTHON3_LIBRARIES=/usr/$(get_libdir)/lib${target}m.so
				)
				break
			fi
		done
		for target in ${targets[@]}; do
			if [[ "${target}" == *2.* ]]; then
				einfo "Selected  ${target}"
				mycmakeargs+=(-DWITH_PYTHON27=$(usex python)
					-DPYTHON27_INCLUDE_DIRS=/usr/include/${target}
					-DPYTHON27_LIBRARIES=/usr/$(get_libdir)/lib${target}.so
				)
				break
			fi
		done
	fi

	cmake-utils_src_configure
}
