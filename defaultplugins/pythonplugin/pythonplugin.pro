# Released as open source by Gabriel Caudrelier
#
# Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com
#
# https://github.com/metrodango/pip3line
#
# Released under BSD 3-Clause see LICENSE for more information

QT       += gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TEMPLATE = lib
CONFIG += plugin debug no_keywords c++14 warn_on

# Comment this out if you want to compile the 2.7 version
CONFIG += CONF_PYTHON_3
#DEFINES += CONF_PYTHON_27

CONF_PYTHON_3 {
    DEFINES += BUILD_PYTHON_3
    TARGET = python3plugin
    unix:!macx {
        LIBS += -lpython3.6m
        INCLUDEPATH +="/usr/include/python3.6m/"
    }

    macx {
        LIBS += -L"/usr/local/Cellar/python3/3.6.4_2/Frameworks/Python.framework/Versions/3.6/lib/" -lpython3.6
        INCLUDEPATH +="/usr/local/Cellar/python3/3.6.4_2/Frameworks/Python.framework/Versions/3.6/include/python3.6m/" INCLUDEPATH
    }

    win32 {
        INCLUDEPATH +="C:\\Python36\\include\\"
        LIBS += -L"C:\\Python36\\libs\\" -lpython36
    }
} else {
    TARGET = python27plugin
    unix {
        LIBS += -lpython2.7
        INCLUDEPATH ="/usr/include/python2.7/" INCLUDEPATH
    }

    win32 {
        INCLUDEPATH +="C:\\Python27\\include\\"
        LIBS += -L"C:\\Python27\\libs\\" -lpython27
    }
}

unix {
    LIBS += -L"../../bin/" -ltransform
}

win32 {
    LIBS += -L"../../lib/" -ltransform
}

INCLUDEPATH += ../../libtransform/
DESTDIR = ../../bin/plugins

DEFINES += PYTHONPLUGIN_LIBRARY

SOURCES += pythonplugin.cpp \
    pythontransform.cpp \
    pythonmodules.cpp

HEADERS += pythonplugin.h\
        pythonplugin_global.h \
    pythontransform.h \
    pythonmodules.h

OTHER_FILES += \
    pythonplugin.json

