# Released as open source by Gabriel Caudrelier
#
# Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com
#
# https://github.com/metrodango/pip3line
#
# Released under AGPL see LICENSE for more information

QT       += svg gui

TARGET = exampleplugin
TEMPLATE = lib
CONFIG += plugin release c++14 warn_on

unix {
    LIBS += -L"../../bin/" -ltransform
}

win32 {
    LIBS += -L"../../lib/" -ltransform
}

INCLUDEPATH += ../../libtransform/
DESTDIR = ../../bin/plugins

DEFINES += EXAMPLEPLUGIN_LIBRARY

SOURCES += exampleplugin.cpp \
        identity.cpp

HEADERS += exampleplugin.h\
        exampleplugin_global.h \
        identity.h

OTHER_FILES += \
    exampleplugin.json

