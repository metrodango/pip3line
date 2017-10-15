/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef PYTHONPLUGIN_H
#define PYTHONPLUGIN_H

#include "pythonplugin_global.h"

#include <transformfactoryplugininterface.h>
#include <QStringList>

class PythonModules;
class Pip3lineCallback;

class PYTHONPLUGINSHARED_EXPORT PythonPlugin : public QObject, public TransformFactoryPluginInterface
{
        Q_OBJECT
#if QT_VERSION >= 0x050000
        Q_PLUGIN_METADATA(IID pluginIID FILE "pythonplugin.json")
#endif
        Q_INTERFACES(TransformFactoryPluginInterface)
    public:

        explicit PythonPlugin();
        ~PythonPlugin();
        void setCallBack(Pip3lineCallback * callback);
        QString pluginName() const;
        QString compiledWithQTversion() const;
        TransformAbstract * getTransform(QString name);
        const QStringList getTransformList(QString typeName);
        QWidget * getConfGui(QWidget * parent);
        const QStringList getTypesList();
        int getLibTransformVersion() const;
        QString pluginVersion() const;
    private:
        QWidget * gui;
        Pip3lineCallback *callback;
        PythonModules * modules;
};

#endif // PYTHONPLUGIN_H
