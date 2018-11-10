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
#include <distorm.h>
#include <QLabel>

DistormPlugin::DistormPlugin()
{
    gui = nullptr;
    callback = nullptr;
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
    TransformAbstract *ta = nullptr;

    if (DistormTransf::id == name) {
        ta = new(std::nothrow) DistormTransf();
        if (ta == nullptr)
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
    if (gui == nullptr) {
        QString info;
        quint32 version = distorm_version();
        info.append(QString("<p>Plugin using DiStorm %1.%2.%3<br>")
                    .arg((version << 8 ) >> 24)
                    .arg((version << 16) >> 24)
                    .arg((version << 24) >> 24));

        info.append("</p>");
        QLabel * label = new(std::nothrow) QLabel(info);
        if (label == nullptr) {
            qFatal("Cannot allocate memory for QLabel (diStorm gui) X{");
        }
        label->setWordWrap(true);
        label->setAlignment(Qt::AlignCenter | Qt::AlignHCenter);
        gui = label;
        connect(gui, &QWidget::destroyed, this, &DistormPlugin::onGuiDelete);
    }
    return gui;
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

void DistormPlugin::onGuiDelete()
{
    gui = nullptr;
}
