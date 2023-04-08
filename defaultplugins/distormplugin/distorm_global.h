/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#ifndef DISTORM_GLOBAL_H
#define DISTORM_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(DISTORM_LIBRARY)
#  define DISTORMSHARED_EXPORT Q_DECL_EXPORT
#else
#  define DISTORMSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // DISTORM_GLOBAL_H
