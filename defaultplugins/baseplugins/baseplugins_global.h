/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef BASEPLUGINS_GLOBAL_H
#define BASEPLUGINS_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(BASEPLUGINS_LIBRARY)
#  define BASEPLUGINSSHARED_EXPORT Q_DECL_EXPORT
#else
#  define BASEPLUGINSSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // BASEPLUGINS_GLOBAL_H
