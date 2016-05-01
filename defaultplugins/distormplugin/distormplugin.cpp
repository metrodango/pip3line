/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "distormplugin.h"
#include "../../version.h"
#include "distormtransf.h"
#include <pip3linecallback.h>
#include <QLabel>

DistormPlugin::DistormPlugin()
{
    gui = NULL;
    callback = NULL;
}

DistormPlugin::~DistormPlugin()
{
}

void DistormPlugin::setCallBack(Pip3lineCallback *ncallback)
{
    callback = ncallback;
}

QString DistormPlugin::pluginName() const
{
    return "Distorm plugin";
}

QString DistormPlugin::compiledWithQTversion() const
{
    return QT_VERSION_STR;
}

TransformAbstract *DistormPlugin::getTransform(QString name)
{
    TransformAbstract *ta = NULL;

    if (DistormTransf::id == name) {
        ta = new(std::nothrow) DistormTransf();
        if (ta == NULL)
            qFatal("Cannot allocate memory for DistormTransf X{");
    }

    return ta;
}

const QStringList DistormPlugin::getTransformList(QString typeName)
{
    QStringList ret;
    if (typeName == DEFAULT_TYPE_MISC) {
        ret.append(DistormTransf::id);
    }

    return ret;
}

QWidget *DistormPlugin::getConfGui(QWidget *)
{
    return NULL;
}

const QStringList DistormPlugin::getTypesList()
{
    return QStringList() << DEFAULT_TYPE_MISC;
}

int DistormPlugin::getLibTransformVersion() const
{
    return LIB_TRANSFORM_VERSION;
}

QString DistormPlugin::pluginVersion() const
{
    return VERSION_STRING;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(distormplugin, DistormPlugin)
#endif
