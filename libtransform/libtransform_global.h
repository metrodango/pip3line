/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef LIBTRANSFORM_GLOBAL_H
#define LIBTRANSFORM_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(LIBTRANSFORM_LIBRARY)
#  define LIBTRANSFORMSHARED_EXPORT Q_DECL_EXPORT
#else
#  define LIBTRANSFORMSHARED_EXPORT Q_DECL_IMPORT
#endif

// need this when declaring global constants, LIBTRANSFORMSHARED_EXPORT is not enough apparently

#if defined(Q_OS_WIN) || defined(Q_CC_NOKIAX86) || defined(Q_CC_RVCT)
#define EXPORTING LIBTRANSFORMSHARED_EXPORT extern
#else
#define EXPORTING LIBTRANSFORMSHARED_EXPORT extern
#endif

#endif // LIBTRANSFORM_GLOBAL_H
