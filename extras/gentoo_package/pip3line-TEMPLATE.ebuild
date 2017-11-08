# $Header: $
#
# Released as open source by Gabriel Caudrelier
#
# Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com
#
# https://github.com/metrodango/pip3line
#
# Released under AGPL see LICENSE for more information
#

EAPI=5

inherit cmake-utils 

DESCRIPTION="Pip3line Raw bytes manipulation and transformation and more"
HOMEPAGE="https://github.com/metrodango/pip3line"
SRC_URI="https://github.com/metrodango/pip3line/files/${P}.tar.gz"

LICENSE="GPL-3"
SLOT="0"
KEYWORDS="amd64 ~x86"
IUSE="openssl python27 python3 qscintilla distorm"

DEPEND="openssl? ( dev-libs/openssl )
		python27? ( dev-lang/python:2.7 )
		python3? ( dev-lang/python:3.2 )
		qscintilla? ( x11-libs/qscintilla[qt5] )
		distorm? ( dev-libs/distorm64 )
		dev-qt/qtcore:5
		dev-qt/qtgui:5
		dev-qt/qtsvg:5
		dev-qt/qtxmlpatterns:5
		dev-qt/qtconcurrent:5
		dev-qt/qtwidgets:5
		dev-qt/qtnetwork:5"

RDEPEND="${DEPEND}"

src_configure() {
    local mycmakeargs=(
		$(cmake-utils_use_with openssl OPENSSL)
		$(cmake-utils_use_with python27 PYTHON27)
		$(cmake-utils_use_with python3 PYTHON3)
		$(cmake-utils_use_with qscintilla SCINTILLA)
		$(cmake-utils_use_with distorm DISTORM)
	)
	cmake-utils_src_configure
}
