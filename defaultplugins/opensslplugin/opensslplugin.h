/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#ifndef OPENSSLPLUGIN_H
#define OPENSSLPLUGIN_H

#include "opensslplugin_global.h"
#include <transformfactoryplugininterface.h>
#include <QMutex>

class OPENSSLPLUGINSHARED_EXPORT OpensslPlugin : public QObject, public TransformFactoryPluginInterface
{
        Q_OBJECT

        Q_PLUGIN_METADATA(IID pluginIID FILE "opensslplugin.json")
        Q_INTERFACES(TransformFactoryPluginInterface)
    public:
        static const QStringList blacklistHash;
        explicit OpensslPlugin();
        ~OpensslPlugin();
        QString pluginName() const;
        QString compiledWithQTversion() const;
        TransformAbstract * getTransform(QString name);
        const QStringList getTransformList(QString type);
        const QStringList getTypesList();
        QWidget * getConfGui(QWidget * parent);
        int getLibTransformVersion() const;
        QString pluginVersion() const;
        void setCallBack(Pip3lineCallback *) {}
    private slots:
        void onGuiDelete();
    private:
        static QMutex hashListLocker;
        QWidget *gui;
};

#endif // OPENSSLPLUGIN_H
