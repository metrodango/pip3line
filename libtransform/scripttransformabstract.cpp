/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "scripttransformabstract.h"
#include "moduletransformwidget.h"
#include <QHash>
#include <QHashIterator>
#include <QDebug>

const QString ScriptTransformAbstract::PROP_SCRIPT = "script";
const QString ScriptTransformAbstract::PROP_MODULE_NAME = "module";
const QString ScriptTransformAbstract::PROP_MODULE_PARAMS = "parameters";

ScriptTransformAbstract::ScriptTransformAbstract(ModulesManagement *mmanagement, const QString &modulename)
{
    type = ModulesManagement::TRANSIENT;
    moduleManagement = mmanagement;
    autoReload = true;

    if (!modulename.isEmpty()) {
        moduleName = modulename;
        type = moduleManagement->getModuleType(modulename);
        moduleFileName = moduleManagement->getModuleFileName(modulename);
    } else {
       // qDebug() << "Modulename is empty";
    }
}

ScriptTransformAbstract::~ScriptTransformAbstract()
{
}

QString ScriptTransformAbstract::name() const
{
    if (type != ModulesManagement::TRANSIENT && !moduleName.isEmpty())
        return moduleName;
    else
        return moduleManagement->getLangName();
}

QHash<QString, QString> ScriptTransformAbstract::getConfiguration()
{
    QHash<QString, QString> properties = TransformAbstract::getConfiguration();
    properties.insert(PROP_MODULE_NAME, moduleName);

    QByteArray serialized;
    QHashIterator<QByteArray, QByteArray> i(parameters);
    while (i.hasNext()) {
        i.next();
        serialized.append(i.key().toBase64()).append(':').append(i.value().toBase64());
        if (i.hasNext())
            serialized.append('|');
    }
    properties.insert(PROP_MODULE_PARAMS, QString::fromUtf8(serialized));
    properties.insert(PROP_SCRIPT, QString::fromUtf8(moduleFileName.toUtf8().toBase64()));
    properties.insert(XMLAUTORELOAD, QString::number(autoReload ?  1 : 0));

    return properties;
}

bool ScriptTransformAbstract::setConfiguration(QHash<QString, QString> propertiesList)
{
    bool res = TransformAbstract::setConfiguration(propertiesList);
    parameters.clear();

    QString fileName = QString::fromUtf8(QByteArray::fromBase64(propertiesList.value(PROP_SCRIPT).toUtf8()));

    if (fileName.isEmpty()) {
        emit error(tr("Module file name is empty, no script loaded"),moduleManagement->getLangName());
        return false;
    }
    moduleName = moduleManagement->getModuleNameFromFile(fileName);
    if (moduleName.isEmpty()) {
        emit error(tr("Module name could not be determined, script not loaded:%1").arg(fileName),moduleManagement->getLangName());
        res = false;
    } else if (moduleManagement->modulesContains(moduleName)) {
        type = moduleManagement->getModuleType(moduleName);
        moduleFileName = fileName;
    } else {
        moduleFileName = fileName;
        moduleName = moduleManagement->addModule(moduleFileName);
        if (!moduleName.isEmpty()) {
            type = moduleManagement->getModuleType(moduleName);
        } else {
            res = false;
        }
    }

    if (!moduleName.isEmpty()) {
        QString parametersList = propertiesList.value(PROP_MODULE_PARAMS, QString());
        if (!parametersList.isEmpty()) {
            QStringList list = parametersList.split("|", Qt::SkipEmptyParts);
            for (int i = 0 ; i < list.size(); i++) {
                QStringList param = list.at(i).split(":");
                if (param.size() != 2) {
                    emit error(tr("Invalid parameter: %1").arg(list.at(i)),moduleManagement->getLangName());
                } else {
                    parameters.insert(QByteArray::fromBase64(param.at(0).toUtf8()),QByteArray::fromBase64(param.at(1).toUtf8()));
                }
            }
        }
    }

    bool ok = false;
    int val = propertiesList.value(XMLAUTORELOAD).toInt(&ok);
    if (!ok || (val != 0 && val != 1)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLAUTORELOAD),moduleManagement->getLangName());
    } else {
        setAutoReload(val == 1);
    }

    return res;
}

QWidget *ScriptTransformAbstract::requestGui(QWidget *parent)
{

    ModuleTransformWidget * widget = new(std::nothrow) ModuleTransformWidget(this, parent);
    if (widget == nullptr) {
        qFatal("Cannot allocate memory for ModuleTransformWidget X{");
    }
    if (loadModule()) { // need the module to be loaded in order to get the configuration
        widget->reloadConf();
    }
    return widget;
}

QString ScriptTransformAbstract::getModuleName() const
{
    return moduleName;
}

QString ScriptTransformAbstract::getModuleFileName() const
{
    return moduleFileName;
}

void ScriptTransformAbstract::setType(ModulesManagement::ModuleType ntype)
{
    if (type == ModulesManagement::AUTO) {
        emit error(tr("Not allowed to change the type for auto loaded modules"),moduleManagement->getLangName());
        return;
    }
    qDebug() << "[ScriptTransformAbstract::setType]";
    if (type != ntype) {
        qDebug() << "[ScriptTransformAbstract::setType] type is different " << ntype;
        if (moduleManagement->setType(moduleName, ntype)) {
            type = moduleManagement->getModuleType(moduleName);
        }
    }
}

ModulesManagement::ModuleType ScriptTransformAbstract::getType() const
{
    return type;
}

QString ScriptTransformAbstract::getScriptDescr() const
{
    return moduleManagement->getLangName();
}

QHash<QByteArray, QByteArray> ScriptTransformAbstract::getParameters() const
{
    return parameters;
}

void ScriptTransformAbstract::setParameters(QHash<QByteArray, QByteArray> newParams)
{
    parameters = newParams;
    emit confUpdated();
}

void ScriptTransformAbstract::setAutoReload(bool val)
{
    if (autoReload != val) {
        autoReload = val;
        if (autoReload)
            loadModule(); // as autoReload is true, this garantee that the module will be reloaded
    }
}

bool ScriptTransformAbstract::isAutoReload() const
{
    return autoReload;
}

