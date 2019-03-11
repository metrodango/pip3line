/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef BASEPLUGINS_H
#define BASEPLUGINS_H

#include "baseplugins_global.h"
#include <transformfactoryplugininterface.h>

class Pip3lineCallback;

class BASEPLUGINSSHARED_EXPORT BasePlugins : public QObject, public TransformFactoryPluginInterface {
        Q_OBJECT

        Q_PLUGIN_METADATA(IID pluginIID FILE "baseplugin.json")

        Q_INTERFACES(TransformFactoryPluginInterface)
    public:
        explicit BasePlugins();
        ~BasePlugins() override;
        QString pluginName() const override;
        QString compiledWithQTversion() const override;
        TransformAbstract * getTransform(QString name) override;
        const QStringList getTransformList(QString typeName) override;
        const QStringList getTypesList() override;
        QWidget * getConfGui(QWidget * parent) override;
        int getLibTransformVersion() const override;
        QString pluginVersion() const override;
        void setCallBack(Pip3lineCallback *) override;
    private:
        TransformAbstract * getTransformFromFile(QString resFile);
        static const QString Base64Url;
        static const QString BinaryNum;
        static const QString OctalNum;
        static const QString HexaNum;
        Pip3lineCallback * callback;
};

#endif // BASEPLUGINS_H
