# Released as open source by Gabriel Caudrelier
#
# Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com
#
# https://github.com/metrodango/pip3line
#
# Released under BSD 3-Clause see LICENSE for more information

QT      += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

VERSION  = 1.2

TARGET   = pip3linecmd
CONFIG  += console release c++14 warn_on
CONFIG  -= app_bundle
DESTDIR  = ../bin
TEMPLATE = app

INCLUDEPATH += "../libtransform"

DEFINES += QT_DEPRECATED_WARNINGS

unix {
    LIBS += -L"../bin" -ltransform
}

win32 {
    LIBS += -L"../lib" -ltransform
}

SOURCES += main.cpp \
    ../tools/processor.cpp \
    masterthread.cpp \
    ../tools/binaryprocessor.cpp \
    ../tools/processingstats.cpp \
    ../tools/textprocessor.cpp

HEADERS += \
    ../tools/processor.h \
    masterthread.h \
    ../tools/binaryprocessor.h \
    ../tools/processingstats.h \
    ../tools/textprocessor.h


unix {
    ROOT_PATH = /usr/local
    target.path = $$ROOT_PATH/bin/
    INSTALLS += target
}

