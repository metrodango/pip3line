# Released as open source by Gabriel Caudrelier
#
# Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com
#
# https://github.com/metrodango/pip3line
#
# Released under AGPL see LICENSE for more information

QT       += svg gui xmlpatterns network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = baseplugins
TEMPLATE = lib
CONFIG += plugin debug c++14 warn_on

DEFINES += QT_DEPRECATED_WARNINGS

unix {
    LIBS += -L"../../bin/" -ltransform
}

win32 {
    LIBS += -L"../../lib/" -ltransform
}

INCLUDEPATH += ../../libtransform/
DESTDIR = ../../bin/plugins

DEFINES += BASEPLUGINS_LIBRARY

SOURCES += baseplugins.cpp \
    xor.cpp \
    urlencode.cpp \
    split.cpp \
    rotx.cpp \
    reverse.cpp \
    regularexp.cpp \
    randomcase.cpp \
    postgresconcat.cpp \
    padding.cpp \
    oracleconcat.cpp \
    mysqlconcatv2.cpp \
    mysqlconcat.cpp \
    mssqlconcat.cpp \
    sha1.cpp \
    md5.cpp \
    md4.cpp \
    javascriptconcat.cpp \
    html.cpp \
    hexencode.cpp \
    cut.cpp \
    charencoding.cpp \
    binary.cpp \
    base64.cpp \
    base32.cpp \
    confgui/xorwidget.cpp \
    confgui/urlencodewidget.cpp \
    confgui/splitwidget.cpp \
    confgui/rotxwidget.cpp \
    confgui/reversewidget.cpp \
    confgui/regularexpwidget.cpp \
    confgui/randomcasewidget.cpp \
    confgui/paddingwidget.cpp \
    confgui/htmlwidget.cpp \
    confgui/hexencodewidget.cpp \
    confgui/cutwidget.cpp \
    confgui/charencodingwidget.cpp \
    confgui/binarywidget.cpp \
    confgui/base64widget.cpp \
    confgui/base32widget.cpp \
    xmlquery.cpp \
    confgui/xmlquerywidget.cpp \
    networkmaskipv6.cpp \
    networkmaskipv4.cpp \
    iptranslateipv4.cpp \
    ciscosecret7.cpp \
    confgui/iptranslateipv4widget.cpp \
    fixprotocol.cpp \
    substitution.cpp \
    confgui/substitutionwidget.cpp \
    numbertochar.cpp \
    hieroglyphy.cpp \
    confgui/hieroglyphywidget.cpp \
    zlib.cpp \
    confgui/zlibwidget.cpp \
    microsofttimestamp.cpp \
    timestamp.cpp \
    confgui/timestampwidget.cpp \
    confgui/microsofttimestampwidget.cpp \
    bytesinteger.cpp \
    confgui/byteintegerwidget.cpp \
    basex.cpp \
    confgui/basexwidget.cpp \
    ntlmssp.cpp \
    confgui/ntlmsspwidget.cpp \
    confgui/numbertocharwidget.cpp \
    bytestofloat.cpp \
    confgui/bytestofloatwdiget.cpp \
    byterot.cpp \
    confgui/byterotwidget.cpp \
    hmactransform.cpp \
    confgui/hmactransformwidget.cpp \
    crc32.cpp \
    confgui/crc32widget.cpp \
    findandreplace.cpp \
    confgui/findandreplacewidget.cpp \
    tds7_enc.cpp \
    jsonvalue.cpp \
    confgui/jsonvaluewidget.cpp


greaterThan(QT_MAJOR_VERSION, 4): SOURCES += sha256.cpp \
    sha512.cpp \
    sha224.cpp \
    sha384.cpp

greaterThan(QT_MAJOR_VERSION, 4): SOURCES += sha3_224.cpp \
    sha3_256.cpp \
    sha3_384.cpp \
    sha3_512.cpp

HEADERS += baseplugins.h\
    baseplugins_global.h\
    xor.h \
    urlencode.h \
    split.h \
    rotx.h \
    reverse.h \
    regularexp.h \
    randomcase.h \
    postgresconcat.h \
    padding.h \
    oracleconcat.h \
    mysqlconcatv2.h \
    mysqlconcat.h \
    mssqlconcat.h \
    sha1.h \
    md5.h \
    md4.h \
    javascriptconcat.h \
    html.h \
    hexencode.h \
    cut.h \
    charencoding.h \
    binary.h \
    base64.h \
    base32.h \
    confgui/xorwidget.h \
    confgui/urlencodewidget.h \
    confgui/splitwidget.h \
    confgui/rotxwidget.h \
    confgui/reversewidget.h \
    confgui/regularexpwidget.h \
    confgui/randomcasewidget.h \
    confgui/paddingwidget.h \
    confgui/htmlwidget.h \
    confgui/hexencodewidget.h \
    confgui/cutwidget.h \
    confgui/charencodingwidget.h \
    confgui/binarywidget.h \
    confgui/base64widget.h \
    confgui/base32widget.h \
    xmlquery.h \
    confgui/xmlquerywidget.h \
    networkmaskipv6.h \
    networkmaskipv4.h \
    iptranslateipv4.h \
    ciscosecret7.h \
    confgui/iptranslateipv4widget.h \
    fixprotocol.h \
    substitution.h \
    confgui/substitutionwidget.h \
    numbertochar.h \
    hieroglyphy.h \
    confgui/hieroglyphywidget.h \
    zlib.h \
    confgui/zlibwidget.h \
    microsofttimestamp.h \
    timestamp.h \
    confgui/timestampwidget.h \
    confgui/microsofttimestampwidget.h \
    bytesinteger.h \
    confgui/byteintegerwidget.h \
    basex.h \
    confgui/basexwidget.h \
    ntlmssp.h \
    confgui/ntlmsspwidget.h \
    confgui/numbertocharwidget.h \
    bytestofloat.h \
    confgui/bytestofloatwdiget.h \
    byterot.h \
    confgui/byterotwidget.h \
    hmactransform.h \
    confgui/hmactransformwidget.h \
    crc32.h \
    confgui/crc32widget.h \
    findandreplace.h \
    confgui/findandreplacewidget.h \
    tds7_enc.h \
    jsonvalue.h \
    confgui/jsonvaluewidget.h

greaterThan(QT_MAJOR_VERSION, 4): HEADERS += sha256.h \
    sha512.h \
    sha224.h \
    sha384.h

greaterThan(QT_MAJOR_VERSION, 4): HEADERS +=     sha3_224.h \
    sha3_256.h \
    sha3_384.h \
    sha3_512.h

FORMS += \
    confgui/xorwidget.ui \
    confgui/urlencodewidget.ui \
    confgui/splitwidget.ui \
    confgui/rotxwidget.ui \
    confgui/reversewidget.ui \
    confgui/regularexpwidget.ui \
    confgui/randomcasewidget.ui \
    confgui/paddingwidget.ui \
    confgui/htmlwidget.ui \
    confgui/hexencodewidget.ui \
    confgui/cutwidget.ui \
    confgui/charencodingwidget.ui \
    confgui/binarywidget.ui \
    confgui/base64widget.ui \
    confgui/base32widget.ui \
    confgui/xmlquerywidget.ui \
    confgui/iptranslateipv4widget.ui \
    confgui/substitutionwidget.ui \
    confgui/hieroglyphywidget.ui \
    confgui/zlibwidget.ui \
    confgui/timestampwidget.ui \
    confgui/microsofttimestampwidget.ui \
    confgui/byteintegerwidget.ui \
    confgui/basexwidget.ui \
    confgui/ntlmsspwidget.ui \
    confgui/numbertocharwidget.ui \
    confgui/bytestofloatwdiget.ui \
    confgui/byterotwidget.ui \
    confgui/hmactransformwidget.ui \
    confgui/crc32widget.ui \
    confgui/findandreplacewidget.ui \
    confgui/jsonvaluewidget.ui

RESOURCES += \
    baseplugins_res.qrc

OTHER_FILES += \
    baseplugin.json
