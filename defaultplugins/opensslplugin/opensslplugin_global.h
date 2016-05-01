/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef OPENSSLPLUGIN_GLOBAL_H
#define OPENSSLPLUGIN_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(OPENSSLPLUGIN_LIBRARY)
#  define OPENSSLPLUGINSHARED_EXPORT Q_DECL_EXPORT
#else
#  define OPENSSLPLUGINSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // OPENSSLPLUGIN_GLOBAL_H
