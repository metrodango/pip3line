/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "modulesmanagement.h"
#include "modulesmanagementwidget.h"
#include "pip3linecallback.h"
#include <QFileInfo>
#include <QDebug>
#include <QWidget>
#include <QDir>
#include <QHashIterator>

const QString ModulesManagement::SETTINGS_USER_MODULES_LIST = "UserModulesList";

ModulesManagement::ModulesManagement(const QString &nlangName, const QString &extension, const QString & baseDir, Pip3lineCallback *ncallback)
{
    gui = nullptr;
    langName = nlangName;
    callback = ncallback;
    baseModulesDirName = baseDir;
    moduleExtension = extension;
}

ModulesManagement::~ModulesManagement()
{
    delete gui;
}

bool ModulesManagement::initialize(const QStringList &initialPaths)
{
    QStringList dirList = callback->pip3lineDirs();
    for (int i = 0; i < dirList.size(); i++) {
        QDir dir(dirList.at(i));
        if (dir.cd(baseModulesDirName)) {
            qDebug() << "Found " << langName << " plugin directory: " << dir.absolutePath();
            modulesPaths.insert(dir.absolutePath(), -1);
        }
    }
    // load modules from default locations
    QHashIterator<QString, int> j(modulesPaths);

    while (j.hasNext()) {
        j.next();
        QDir mDir(j.key());
        QStringList filters;
        filters << QString(moduleExtension).prepend("*");
        mDir.setNameFilters(filters);
        foreach (QString fileName, mDir.entryList(QDir::Files)) {
            QString modulePath = mDir.absolutePath().append(QDir::separator()).append(fileName);
            if (checkModuleNameAndPath(modulePath, getModuleNameFromFile(modulePath))) {
                callback->logStatus(tr("Registering module file %1 from %2").arg(fileName).arg(mDir.absolutePath()));
                addModule(modulePath, AUTO);
            } else {
                callback->logStatus(tr("Skipping module file %1 from %2").arg(fileName).arg(mDir.absolutePath()));
            }
        }
    }

    // adding default SysPath
    for (int i = 0; i < initialPaths.size(); i++)
        modulesPaths.insert(initialPaths.at(i),-1);

    // retrieving modules from persistent storage
    QStringList list = callback->getParameter(SETTINGS_USER_MODULES_LIST).toStringList();
    for (int i = 0; i < list.size(); i++) {
        addModule(list.at(i),PERSISTENT);
    }

    savePersistentModules(); // to remove any erronous module from the persistent storage

    return true;
}

QStringList ModulesManagement::getPathsList()
{
    return modulesPaths.keys();
}

QStringList ModulesManagement::getModulesList()
{
    return modulesList.keys();
}

QStringList ModulesManagement::getRegisteredModule()
{
    QStringList list;
    QHashIterator<QString, ModuleProperties> i(modulesList);
     while (i.hasNext()) {
         i.next();
         if (i.value().type != TRANSIENT)
            list << i.key();
     }
     return list;
}

bool ModulesManagement::setType(const QString &name, ModulesManagement::ModuleType type)
{
//    qDebug() << "[ModulesManagement::setType] " << name << type;
    if (modulesList.contains(name)) {
        if (type == AUTO) {
            callback->logError(tr("[ModulesManagement::setType] - Cannot change the type to AUTO for %1 T_T").arg(name));
            return false;
        }
        ModuleType previous = modulesList.value(name).type;
        if (previous == type) // if there is no change, just return
            return true;
        else if (previous == AUTO) {
            callback->logError(tr("[ModulesManagement::setType] - Cannot change the type of %1. Module loaded from default location T_T").arg(name));
            return false;
        }
        ModuleProperties prop =  modulesList.value(name);
        prop.type = type;
        modulesList.insert(name, prop);
        savePersistentModules();
        callback->notifyNewTransform(); // need to renew the list in every remaining case
    } else {
        callback->logError(tr("[ModulesManagement::setType] - No module named \"%1\" T_T").arg(name));
        return false;
    }

    return true;
}

void ModulesManagement::removeModule(const QString &name)
{
    if (modulesList.contains(name)) {
        ModuleType moduleType = modulesList.value(name).type;
        if (moduleType == AUTO) {
            callback->logError(tr("[ModulesManagement::removeModule] - Cannot remove module \"%1\", this module was loaded from default a location.").arg(name));
            return;
        }
        modulesList.remove(name);
        if (moduleType == PERSISTENT) {
            savePersistentModules();
            callback->notifyNewTransform();
        }
        emit modulesUpdated();
    } else {
        callback->logWarning(tr("[ModulesManagement::removeModule] - No module \"%1\"").arg(name));
    }
}

ModulesManagement::ModuleType ModulesManagement::getModuleType(const QString &name)
{
    if (modulesList.contains(name)) {
        return modulesList.value(name).type;
    } else {
        callback->logError(tr("[ModulesManagement::getModuleType] - No module \"%1\"").arg(name));
    }
    return TRANSIENT;
}

QString ModulesManagement::getModuleFileName(const QString &name)
{
    QString fileName;
    if (modulesList.contains(name)) {
        fileName = modulesList.value(name).fileName;
    } else {
        callback->logError(tr("[ModulesManagement::getModuleFileName] - No module \"%1\"").arg(name));
    }
    return fileName;
}

bool ModulesManagement::modulesContains(const QString &name)
{
    if (modulesList.contains(name)) {
        return modulesList.value(name).type != TRANSIENT;
    } else {
        return false;
    }
    return modulesList.contains(name);
}

bool ModulesManagement::isRegistered(const QString &name)
{
    if (modulesList.contains(name)) {
        return modulesList.value(name).type != TRANSIENT;
    }
    return false;
}

QString ModulesManagement::getExtension() const
{
    return moduleExtension;
}

QString ModulesManagement::getLangName() const
{
    return langName;
}

QString ModulesManagement::getModuleNameFromFile(QString fileName)
{
    QString moduleName;
    QFileInfo file(fileName);
    if (file.exists()) {
        moduleName = file.fileName();
        if (moduleExtension.size() > 0)
         moduleName.chop(moduleExtension.size());
        else
            callback->logError(tr("[ModulesManagement::getModuleNameFromFile] Module extension is undefined T_T"));
    }  else {
        callback->logError(tr("[ModulesManagement::getModuleNameFromFile] File %1 does not exist").arg(fileName));
    }

    return moduleName;
}

QWidget *ModulesManagement::getGui(QWidget *parent)
{
    if (gui == nullptr) {
        gui = new(std::nothrow) ModulesManagementWidget(this, parent);
        if (gui == nullptr) {
            qFatal("Cannot allocate memory for ModulesManagementWidget X{");
        }

        connect(gui, &ModulesManagementWidget::destroyed, this, &ModulesManagement::onGuiDelete);
    }
    return gui;
}

QString ModulesManagement::getInfos()
{
    return QString();
}

QString ModulesManagement::addModule(QString fileName, ModulesManagement::ModuleType type)
{
//    qDebug() << "[ModulesManagement::addModule]" << fileName;
    QString moduleName;
    QFileInfo file(fileName);
    if (file.exists()) {
        moduleName = file.fileName();
        moduleName.chop(moduleExtension.size());
        if (modulesList.contains(moduleName)) {
         //   qDebug() << QString("[ModulesManagement::addModule] A module named \"%1\" was already loaded, skipping.").arg(moduleName);
        } else {
            QString path = file.absolutePath();
            if (!modulesPaths.contains(path)) {
                modulesPaths.insert(path, 1);
                emit pathsUpdated();
            } else if (modulesPaths.value(path) > 0){
                modulesPaths.insert(path, modulesPaths.value(path) + 1);
            }
            ModuleProperties prop;
            prop.fileName = fileName;
            prop.type = type;
            prop.autoReload = true;
            modulesList.insert(moduleName,prop);
            if (type == PERSISTENT)
                savePersistentModules();
            emit modulesUpdated();
            callback->notifyNewTransform();
        }
    } else {
        callback->logError(tr("[ModulesManagement::addModule] File %1 does not exist").arg(fileName));
    }
    return moduleName;
}

void ModulesManagement::savePersistentModules()
{
    QHashIterator<QString, ModuleProperties> i(modulesList);
    QStringList list;
    while (i.hasNext()) {
        i.next();
        if (i.value().type == PERSISTENT) {
            list << i.value().fileName;
        }
    }

    callback->saveParameter(SETTINGS_USER_MODULES_LIST,list );
}


void ModulesManagement::onGuiDelete()
{
    gui = nullptr;
}
