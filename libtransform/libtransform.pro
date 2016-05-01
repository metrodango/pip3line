# Released as open source by Gabriel Caudrelier
#
# Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com
#
# https://github.com/metrodango/pip3line
#
# Released under AGPL see LICENSE for more information


QT       += svg concurrent
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = transform
TEMPLATE = lib
CONFIG += debug plugin
DEFINES += LIBTRANSFORM_LIBRARY

QMAKE_CXXFLAGS += -std=c++11

unix {
    DESTDIR = ../bin
}

win32 {
    DESTDIR = ../lib
}

SOURCES += transformmgmt.cpp \
    transformabstract.cpp \
    pip3linecallback.cpp \
    transformchain.cpp \
    composedtransform.cpp \
    modulesmanagementwidget.cpp \
    modulesmanagement.cpp \
    moduletransformwidget.cpp \
    scripttransformabstract.cpp \
    deleteablelistitem.cpp \
    threadedprocessor.cpp \
    commonstrings.cpp \
    hexwidget.cpp

HEADERS += \
    transformmgmt.h \
    transformabstract.h \
    transformfactoryplugininterface.h \
    commonstrings.h \
    libtransform_global.h \
    pip3linecallback.h \
    ../version.h \
    transformchain.h \
    composedtransform.h \
    modulesmanagementwidget.h \
    modulesmanagement.h \
    moduletransformwidget.h \
    scripttransformabstract.h \
    deleteablelistitem.h \
    threadedprocessor.h \
    hexwidget.h

FORMS += \
    modulesmanagementwidget.ui \
    moduletransformwidget.ui \
    deleteablelistitem.ui \
    hexwidget.ui

