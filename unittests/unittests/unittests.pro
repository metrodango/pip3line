# Released as open source by Gabriel Caudrelier
#
# Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com
#
# https://github.com/metrodango/pip3line
#
# Released under AGPL see LICENSE for more information

QT       += network svg xml xmlpatterns testlib

TARGET = tst_unitteststest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
CONFIG += CONF_ADDR_SANITIZER

SOURCES += tst_unitteststest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

LIBS += -L"../../bin/" -ltransform
INCLUDEPATH += ../../libtransform
DESTDIR = $$PWD/../../bin

unix {
    CONF_ADDR_SANITIZER {
        QMAKE_CXXFLAGS += -g -O1 -fsanitize=address -fno-omit-frame-pointer
        QMAKE_LFLAGS += -fsanitize=address
    }
}
