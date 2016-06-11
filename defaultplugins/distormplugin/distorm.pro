# Released as open source by Gabriel Caudrelier
#
# Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com
#
# https://github.com/metrodango/pip3line
#
# Released under AGPL see LICENSE for more information

QT       += svg
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = distormplugin
TEMPLATE = lib
CONFIG += plugin c++11 warn_on

DEFINES += DISTORM_LIBRARY

SOURCES += distormplugin.cpp \
    distormwidget.cpp \
    distormtransf.cpp

HEADERS += distormplugin.h\
        distorm_global.h \
    distormwidget.h \
    distormtransf.h

unix {
    LIBS += -L"../../bin/" -ltransform -L"../../../distorm/build/lib/distorm3/" -ldistorm3
}

win32 {
    QMAKE_LFLAGS_WINDOWS += /FS
    LIBS += -L"../../lib/" -ltransform -L"../../../distorm/" -ldistorm
}

INCLUDEPATH += ../../libtransform/ ../../../distorm/include/
DESTDIR = ../../bin/plugins

OTHER_FILES += \
    distormplugin.json

FORMS += \
    distormwidget.ui
