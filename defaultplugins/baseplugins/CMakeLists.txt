####
# Released as open source by Gabriel Caudrelier
#
# Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com
#
# https://github.com/metrodango/pip3line
#
# Released under AGPL see LICENSE for more information
####

message ( STATUS "Configuring the base plugin --" )
project( baseplugins )
set ( CMAKE_BUILD_TYPE Release )
add_definitions ( -Wall -DBASEPLUGINS_LIBRARY)


set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Widgets)
find_package(Qt5Core)
find_package(Qt5Svg)
find_package(Qt5Network)
find_package(Qt5XmlPatterns)

include_directories (
	../../libtransform/)

if (WIN32)
link_directories (
	${PROJECT_SOURCE_DIR}/../../lib
	)
endif(WIN32)

set ( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../../bin/plugins )

set ( baseplugins_HDRS
	baseplugins_global.h
	md4.h
	md5.h
	sha1.h
	baseplugins.h
	substitution.h
	numbertochar.h
	xor.h
	urlencode.h
	split.h
	rotx.h
	reverse.h
	regularexp.h
	randomcase.h
	postgresconcat.h
	padding.h
	oracleconcat.h
	mysqlconcatv2.h
	mysqlconcat.h
	mssqlconcat.h
	javascriptconcat.h
	html.h
	hexencode.h
	cut.h
	charencoding.h
	binary.h
	base64.h
	base32.h
	basex.h
	iptranslateipv4.h
	networkmaskipv4.h
	networkmaskipv6.h
	ciscosecret7.h
	xmlquery.h
	fixprotocol.h
	hieroglyphy.h
	zlib.h
	timestamp.h
	microsofttimestamp.h
	bytesinteger.h
	ntlmssp.h
	bytestofloat.h
	byterot.h
	hmactransform.h
	findandreplace.h
	crc32.h
	tds7_enc.h
	confgui/xorwidget.h
	confgui/urlencodewidget.h
	confgui/splitwidget.h
	confgui/rotxwidget.h
	confgui/reversewidget.h
	confgui/regularexpwidget.h
	confgui/randomcasewidget.h
	confgui/paddingwidget.h
	confgui/htmlwidget.h
	confgui/hexencodewidget.h
	confgui/cutwidget.h
	confgui/charencodingwidget.h
	confgui/binarywidget.h
	confgui/base64widget.h
	confgui/base32widget.h
	confgui/xmlquerywidget.h
	confgui/iptranslateipv4widget.h
	confgui/substitutionwidget.h
	confgui/hieroglyphywidget.h
	confgui/zlibwidget.h
	confgui/timestampwidget.h
	confgui/microsofttimestampwidget.h
	confgui/byteintegerwidget.h
	confgui/basexwidget.h
	confgui/ntlmsspwidget.h
	confgui/numbertocharwidget.h
	confgui/bytestofloatwdiget.h
	confgui/byterotwidget.h
	confgui/hmactransformwidget.h
	confgui/crc32widget.h
	confgui/findandreplacewidget.h
	)

set ( baseplugins_SRCS
	md5.cpp
	md4.cpp
	sha1.cpp
	baseplugins.cpp
	substitution.cpp
	numbertochar.cpp
	xor.cpp
	urlencode.cpp
	split.cpp
	rotx.cpp
	reverse.cpp
	regularexp.cpp
	randomcase.cpp
	postgresconcat.cpp
	padding.cpp
	oracleconcat.cpp
	mysqlconcatv2.cpp
	mysqlconcat.cpp
	mssqlconcat.cpp
	javascriptconcat.cpp
	html.cpp
	hexencode.cpp
	cut.cpp
	charencoding.cpp
	binary.cpp
	base64.cpp
	base32.cpp
	basex.cpp
	iptranslateipv4.cpp
	networkmaskipv4.cpp
	networkmaskipv6.cpp
	ciscosecret7.cpp
	xmlquery.cpp
	fixprotocol.cpp
	hieroglyphy.cpp
	zlib.cpp
	timestamp.cpp
	microsofttimestamp.cpp
	bytesinteger.cpp
	ntlmssp.cpp
	bytestofloat.cpp
	byterot.cpp
	hmactransform.cpp
	crc32.cpp
	tds7_enc.cpp
	findandreplace.cpp
	confgui/xorwidget.cpp
	confgui/urlencodewidget.cpp
	confgui/splitwidget.cpp
	confgui/rotxwidget.cpp
	confgui/reversewidget.cpp
	confgui/regularexpwidget.cpp
	confgui/randomcasewidget.cpp
	confgui/paddingwidget.cpp
	confgui/htmlwidget.cpp
	confgui/hexencodewidget.cpp
	confgui/cutwidget.cpp
	confgui/charencodingwidget.cpp
	confgui/binarywidget.cpp
	confgui/base64widget.cpp
	confgui/base32widget.cpp
	confgui/xmlquerywidget.cpp
	confgui/iptranslateipv4widget.cpp
	confgui/substitutionwidget.cpp
	confgui/hieroglyphywidget.cpp
	confgui/zlibwidget.cpp
	confgui/timestampwidget.cpp
	confgui/microsofttimestampwidget.cpp
	confgui/byteintegerwidget.cpp
	confgui/basexwidget.cpp
	confgui/ntlmsspwidget.cpp
	confgui/numbertocharwidget.cpp
	confgui/bytestofloatwdiget.cpp
	confgui/byterotwidget.cpp
	confgui/hmactransformwidget.cpp
	confgui/crc32widget.cpp
	confgui/findandreplacewidget.cpp
	)

set ( baseplugins_UIS
	confgui/xorwidget.ui
	confgui/urlencodewidget.ui
	confgui/splitwidget.ui
	confgui/rotxwidget.ui
	confgui/reversewidget.ui
	confgui/regularexpwidget.ui
	confgui/randomcasewidget.ui
	confgui/paddingwidget.ui
	confgui/htmlwidget.ui
	confgui/hexencodewidget.ui
	confgui/cutwidget.ui
	confgui/charencodingwidget.ui
	confgui/binarywidget.ui
	confgui/base64widget.ui
	confgui/base32widget.ui
	confgui/xmlquerywidget.ui
	confgui/iptranslateipv4widget.ui
	confgui/substitutionwidget.ui
	confgui/hieroglyphywidget.ui
	confgui/zlibwidget.ui
	confgui/timestampwidget.ui
	confgui/microsofttimestampwidget.ui
	confgui/byteintegerwidget.ui
	confgui/basexwidget.ui
	confgui/ntlmsspwidget.ui
	confgui/numbertocharwidget.ui
	confgui/bytestofloatwdiget.ui
	confgui/byterotwidget.ui
	confgui/hmactransformwidget.ui
	confgui/crc32widget.ui
	confgui/findandreplacewidget.ui
	)

set ( baseplugins_HDRS
    ${baseplugins_HDRS}
    sha224.h
    sha256.h
    sha384.h
    sha512.h
    sha3_224.h
    sha3_256.h
    sha3_384.h
    sha3_512.h
)

set ( baseplugins_SRCS
    ${baseplugins_SRCS}
    sha224.cpp
    sha256.cpp
    sha384.cpp
    sha512.cpp
    sha3_224.cpp
    sha3_256.cpp
    sha3_384.cpp
    sha3_512.cpp
)
	
set ( baseplugins_RSCS
	baseplugins_res.qrc
	)

qt5_wrap_ui(UIS ${baseplugins_UIS})
QT5_ADD_RESOURCES(RSCS ${baseplugins_RSCS})

set ( EXTERN_LIB transform )

add_library ( baseplugins MODULE ${baseplugins_SRCS} ${UIS} ${RSCS} )

qt5_use_modules( baseplugins Widgets Svg Network XmlPatterns )

target_link_libraries ( baseplugins ${EXTERN_LIB} )

IF(UNIX)
	install (TARGETS baseplugins DESTINATION lib/pip3line/
		PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
		GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
ENDIF(UNIX)

