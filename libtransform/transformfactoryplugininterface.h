/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef TRANSFORMFACTORYPLUGININTERFACE_H
#define TRANSFORMFACTORYPLUGININTERFACE_H

#include <QtPlugin>

QT_BEGIN_NAMESPACE
class QString;
class QStringList;
class QWidget;
class Pip3lineCallback;
class TransformAbstract;

class TransformFactoryPluginInterface
{
    public:
        virtual ~TransformFactoryPluginInterface() {}
        virtual void setCallBack(Pip3lineCallback *) = 0;
        virtual QString pluginName() const = 0;
        virtual TransformAbstract * getTransform(QString name) = 0;
        virtual const QStringList getTransformList(QString typeName) = 0;
        virtual const QStringList getTypesList() = 0;
        virtual QWidget * getConfGui(QWidget * parent) = 0;
        virtual QString compiledWithQTversion() const = 0;
        virtual int getLibTransformVersion() const = 0;
        virtual QString pluginVersion() const = 0;
};

#define pluginIID "com.pip3line/1.2"
Q_DECLARE_INTERFACE(TransformFactoryPluginInterface, pluginIID)
QT_END_NAMESPACE

#endif // TRANSFORMFACTORYPLUGININTERFACE_H
