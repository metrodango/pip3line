# Released as open source by Gabriel Caudrelier
#
# Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com
#
# https://github.com/metrodango/pip3line
#
# Released under BSD 3-Clause see LICENSE for more information


QT       += svg concurrent widgets

TARGET = transform
TEMPLATE = lib
CONFIG += debug plugin c++14 warn_on
DEFINES += LIBTRANSFORM_LIBRARY

#CONFIG += CONF_ADDR_SANITIZER
#CONFIG += CONF_THREAD_SANITIZER

DEFINES += QT_DEPRECATED_WARNINGS

unix {

    CONF_ADDR_SANITIZER {
        QMAKE_CXXFLAGS += -fsanitize=address
        QMAKE_LDFLAGS += -fsanitize=address
        LIBS += -L"/usr/lib/clang/5.0.0/lib/linux" -lclang_rt.asan_cxx-x86_64
    }

    CONF_THREAD_SANITIZER {
        QMAKE_CXXFLAGS += -fsanitize=thread
        LIBS += -L"/usr/lib/clang/5.0.0/lib/linux" -ltsan
    }
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

