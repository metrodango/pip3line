/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#ifndef MODULESMANAGEMENT_H
#define MODULESMANAGEMENT_H

#include <QList>
#include <QStringList>
#include <QObject>
#include <QHash>

#include "libtransform_global.h"

class ModulesManagementWidget;
class Pip3lineCallback;
class QWidget;

class LIBTRANSFORMSHARED_EXPORT ModulesManagement : public QObject
{
        Q_OBJECT
    public:
        enum ModuleType {TRANSIENT = 0, AUTO = 1, PERSISTENT};
        struct ModuleProperties {
                QString fileName;
                ModuleType type;
                bool autoReload;
        };
        explicit ModulesManagement(const QString &langName, const QString &extension, const QString & baseDir, Pip3lineCallback *callback);
        ~ModulesManagement();

        QStringList getPathsList();
        QStringList getModulesList();
        QStringList getRegisteredModule();

        QString addModule(QString fileName, ModulesManagement::ModuleType type = TRANSIENT);
        virtual bool unloadModules(QString identifier) = 0;
        virtual void unloadModules() = 0;

        bool setType(const QString &name, ModulesManagement::ModuleType type);
        void removeModule(const QString &name);
        void removePersistentFile(const QString &fileName);

        ModuleType getModuleType(const QString &name);
        QString getModuleFileName(const QString &name);

        bool modulesContains(const QString &name);
        bool isRegistered(const QString &name);

        QString getExtension() const;
        QString getLangName() const;

        QString getModuleNameFromFile(QString fileName);

        QWidget *getGui(QWidget * parent);

        virtual QString getInfos();

    Q_SIGNALS:
        void pathsUpdated();
        void modulesUpdated();
    protected:
        bool initialize(const QStringList &initialPaths);
        virtual bool checkModuleNameAndPath(QString modulePath, QString moduleName) = 0;
        QHash<QString, int> modulesPaths;
        QHash<QString,ModuleProperties> modulesList;
        QString moduleExtension;
        Pip3lineCallback *callback;
        QString baseModulesDirName;
        ModulesManagementWidget *gui;
        QString langName;
    private Q_SLOTS:
        void onGuiDelete();
    private:
        Q_DISABLE_COPY(ModulesManagement)
        static const QString SETTINGS_USER_MODULES_LIST;
        void savePersistentModules();
};

#endif // MODULESMANAGEMENT_H
