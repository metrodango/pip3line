###
# Released as open source by Gabriel Caudrelier
#
# Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com
#
# https://github.com/metrodango/pip3line
#
# Released under BSD 3-Clause see LICENSE for more information
###

cmake_minimum_required(VERSION 3.5)
message ( STATUS "Configuring the base plugin --" )
project( baseplugins )
#set ( CMAKE_BUILD_TYPE Release )
add_definitions ( -Wall -DBASEPLUGINS_LIBRARY)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Svg Network Concurrent)

include_directories (
    ../../libtransform/
)

if (WIN32)
    link_directories (
        ${PROJECT_SOURCE_DIR}/../../lib
    )
endif(WIN32)

set ( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../../bin/plugins )

set ( baseplugins_HDRS
    base32.h
    base64.h
    baseplugins.h
    baseplugins_global.h
    basex.h
    binary.h
    byterot.h
    bytesinteger.h
    bytestofloat.h
    charencoding.h
    ciscosecret7.h
    crc32.h
    cut.h
    findandreplace.h
    fixprotocol.h
    hexencode.h
    hieroglyphy.h
    hmactransform.h
    html.h
    iptranslateipv4.h
    javascriptconcat.h
    jsonvalue.h
    md4.h
    md5.h
    microsofttimestamp.h
    mssqlconcat.h
    mysqlconcat.h
    mysqlconcatv2.h
    networkmaskipv4.h
    networkmaskipv6.h
    ntlmssp.h
    numbertochar.h
    oracleconcat.h
    padding.h
    postgresconcat.h
    randomcase.h
    regularexp.h
    reverse.h
    rotx.h
    sha1.h
    split.h
    substitution.h
    tds7_enc.h
    timestamp.h
    urlencode.h
    xmlquery.h
    xor.h
    zlib.h
    confgui/base32widget.h
    confgui/base64widget.h
    confgui/basexwidget.h
    confgui/binarywidget.h
    confgui/byteintegerwidget.h
    confgui/byterotwidget.h
    confgui/bytestofloatwdiget.h
    confgui/charencodingwidget.h
    confgui/crc32widget.h
    confgui/cutwidget.h
    confgui/findandreplacewidget.h
    confgui/hexencodewidget.h
    confgui/hieroglyphywidget.h
    confgui/hmactransformwidget.h
    confgui/htmlwidget.h
    confgui/iptranslateipv4widget.h
    confgui/jsonvaluewidget.h
    confgui/microsofttimestampwidget.h
    confgui/ntlmsspwidget.h
    confgui/numbertocharwidget.h
    confgui/paddingwidget.h
    confgui/randomcasewidget.h
    confgui/regularexpwidget.h
    confgui/reversewidget.h
    confgui/rotxwidget.h
    confgui/splitwidget.h
    confgui/substitutionwidget.h
    confgui/timestampwidget.h
    confgui/urlencodewidget.h
    confgui/xmlquerywidget.h
    confgui/xorwidget.h
    confgui/zlibwidget.h
    )

set ( baseplugins_SRCS
    base32.cpp
    base64.cpp
    baseplugins.cpp
    basex.cpp
    binary.cpp
    byterot.cpp
    bytesinteger.cpp
    bytestofloat.cpp
    charencoding.cpp
    ciscosecret7.cpp
    crc32.cpp
    cut.cpp
    findandreplace.cpp
    fixprotocol.cpp
    hexencode.cpp
    hieroglyphy.cpp
    hmactransform.cpp
    html.cpp
    iptranslateipv4.cpp
    javascriptconcat.cpp
    jsonvalue.cpp
    md4.cpp
    md5.cpp
    microsofttimestamp.cpp
    mssqlconcat.cpp
    mysqlconcat.cpp
    mysqlconcatv2.cpp
    networkmaskipv4.cpp
    networkmaskipv6.cpp
    ntlmssp.cpp
    numbertochar.cpp
    oracleconcat.cpp
    padding.cpp
    postgresconcat.cpp
    randomcase.cpp
    regularexp.cpp
    reverse.cpp
    rotx.cpp
    sha1.cpp
    split.cpp
    substitution.cpp
    tds7_enc.cpp
    timestamp.cpp
    urlencode.cpp
    xmlquery.cpp
    xor.cpp
    zlib.cpp
    confgui/base32widget.cpp
    confgui/base64widget.cpp
    confgui/basexwidget.cpp
    confgui/binarywidget.cpp
    confgui/byteintegerwidget.cpp
    confgui/byterotwidget.cpp
    confgui/bytestofloatwdiget.cpp
    confgui/charencodingwidget.cpp
    confgui/crc32widget.cpp
    confgui/cutwidget.cpp
    confgui/findandreplacewidget.cpp
    confgui/hexencodewidget.cpp
    confgui/hieroglyphywidget.cpp
    confgui/hmactransformwidget.cpp
    confgui/htmlwidget.cpp
    confgui/iptranslateipv4widget.cpp
    confgui/jsonvaluewidget.cpp
    confgui/microsofttimestampwidget.cpp
    confgui/ntlmsspwidget.cpp
    confgui/numbertocharwidget.cpp
    confgui/paddingwidget.cpp
    confgui/randomcasewidget.cpp
    confgui/regularexpwidget.cpp
    confgui/reversewidget.cpp
    confgui/rotxwidget.cpp
    confgui/splitwidget.cpp
    confgui/substitutionwidget.cpp
    confgui/timestampwidget.cpp
    confgui/urlencodewidget.cpp
    confgui/xmlquerywidget.cpp
    confgui/xorwidget.cpp
    confgui/zlibwidget.cpp
    )

set ( baseplugins_UIS
    confgui/base32widget.ui
    confgui/base64widget.ui
    confgui/basexwidget.ui
    confgui/binarywidget.ui
    confgui/byteintegerwidget.ui
    confgui/byterotwidget.ui
    confgui/bytestofloatwdiget.ui
    confgui/charencodingwidget.ui
    confgui/crc32widget.ui
    confgui/cutwidget.ui
    confgui/findandreplacewidget.ui
    confgui/hexencodewidget.ui
    confgui/hieroglyphywidget.ui
    confgui/hmactransformwidget.ui
    confgui/htmlwidget.ui
    confgui/iptranslateipv4widget.ui
    confgui/jsonvaluewidget.ui
    confgui/microsofttimestampwidget.ui
    confgui/ntlmsspwidget.ui
    confgui/numbertocharwidget.ui
    confgui/paddingwidget.ui
    confgui/randomcasewidget.ui
    confgui/regularexpwidget.ui
    confgui/reversewidget.ui
    confgui/rotxwidget.ui
    confgui/splitwidget.ui
    confgui/substitutionwidget.ui
    confgui/timestampwidget.ui
    confgui/urlencodewidget.ui
    confgui/xmlquerywidget.ui
    confgui/xorwidget.ui
    confgui/zlibwidget.ui
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

set ( EXTERN_LIB transform )

add_library ( baseplugins MODULE ${baseplugins_SRCS} ${baseplugins_UIS} ${baseplugins_RSCS} )

if (Qt6_FOUND)
    target_link_libraries ( baseplugins PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Svg Qt${QT_VERSION_MAJOR}::Network ${EXTERN_LIB} )
else()
    target_link_libraries ( baseplugins PRIVATE Qt5::Widgets Qt5::Svg Qt5::Network Qt5::XmlPatterns ${EXTERN_LIB} )
endif()

IF(UNIX)
    include( GNUInstallDirs )
    install (TARGETS baseplugins DESTINATION ${CMAKE_INSTALL_LIBDIR}/pip3line/
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
ENDIF(UNIX)

