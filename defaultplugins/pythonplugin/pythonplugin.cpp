/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "pythonplugin.h"
#include "pythontransform.h"
#include "../../version.h"
#include "pythonmodules.h"
#include <QDebug>
#include <transformmgmt.h>
#include <pip3linecallback.h>
#include <modulesmanagement.h>
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <pip3linecallback.h>
#include <modulesmanagement.h>
#include <QObject>

PythonPlugin::PythonPlugin()
{
    callback = nullptr;
    gui = nullptr;
    modules = nullptr;

    qDebug() << "Created " << this;
}

PythonPlugin::~PythonPlugin()
{
    qDebug() << "Destroying " << this;

    delete modules;
}

void PythonPlugin::setCallBack(Pip3lineCallback *ncallback)
{
    callback = ncallback;

    modules = new(std::nothrow) PythonModules(PythonTransform::id, callback);

    if (modules == nullptr) {
       qFatal("Cannot allocate memory for ModulesManagement (PythonPlugin) X{");
    }
    modules->initialize();
}

QString PythonPlugin::pluginName() const
{

    return QString("%1 plugin").arg(PythonModules::PYTHON_TYPE);
}

QString PythonPlugin::compiledWithQTversion() const
{
    return QT_VERSION_STR;
}

TransformAbstract *PythonPlugin::getTransform(QString name)
{
    PythonTransform *ta = nullptr;

    if (name == PythonTransform::id) {
        ta = new(std::nothrow) PythonTransform(modules);
        if (ta == nullptr) {
           qFatal("Cannot allocate memory for Python (PythonPlugin 1) X{");
        }
    }
    else if (modules->isRegistered(name)) {
        ta = new(std::nothrow) PythonTransform(modules, name);
        if (ta == nullptr) {
           qFatal("Cannot allocate memory for Python (PythonPlugin 2) X{");
        }
    }

    return ta;
}

const QStringList PythonPlugin::getTransformList(QString typeName)
{
    QStringList ret;
    if (typeName == PythonModules::PYTHON_TYPE) {
        ret.append(PythonTransform::id);
        ret.append(modules->getRegisteredModule());
    }
    return ret;
}

QWidget *PythonPlugin::getConfGui(QWidget *parent)
{
    return modules->getGui(parent);
}

const QStringList PythonPlugin::getTypesList()
{
    return QStringList() << PythonModules::PYTHON_TYPE;
}

int PythonPlugin::getLibTransformVersion() const
{
    return LIB_TRANSFORM_VERSION;
}

QString PythonPlugin::pluginVersion() const
{
    return VERSION_STRING;
}
