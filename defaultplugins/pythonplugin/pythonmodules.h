/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#ifndef PYTHONMODULES_H
#define PYTHONMODULES_H

#include "modulesmanagement.h"
#include <QHash>



class Pip3lineCallback;
typedef struct _ts PyThreadState;
typedef struct _object PyObject;

class PythonModules : public ModulesManagement
{
        Q_OBJECT
    public:
        static const QString PYTHON_TYPE;
        static const char * MAIN_FUNCTION_NAME;
        static const char * ISTWOWAY_ATTR_NAME;
        static const char * INBOUND_ATTR_NAME;
        static const char * PARAMS_ATTR_NAME;
        static const char * PARAMS_NAMES_ATTR_NAME;
        explicit PythonModules(QString name, Pip3lineCallback *callback);
        ~PythonModules();
        bool initialize();
        bool checkPyError();
        QString getLastError();
        PyObject * loadModule(QString modulePath, bool reload = false, bool *firstLoad = nullptr);
        bool unloadModules(QString modulePath);
        void unloadModules();
        QString getInfos();
    private Q_SLOTS:
        void updatePath();
        QString retrievePythonErrors();
    private:
        static const QString BASE_SCRIPTS_DIR;
        static const QString PYTHON_EXTENSION;
        static const char * MODULE_FILE_PATH_STR;
#ifdef BUILD_PYTHON_3
        static wchar_t PROG_NAME[];
#else
        static char PROG_NAME[];
#endif
        PyObject *pyGetValFunc;
        PyObject *pyTruncateFunc;
        PyObject *pySeekFunc;
        PyObject *pyStringIO;
        QStringList defaultsysPath;
        PyThreadState *pymainstate;

        QHash<QString, PyObject*> modulesPath;

        void cleaningPyObjs();
        QString getRuntimeVersion();
        QString pyStringToQtString(PyObject *strPyObj);
        bool checkPyObject(PyObject *obj);
        void settingUpStderr();
        void disablingSIGINT();
        bool checkModuleNameAndPath(QString modulePath, QString moduleName);
        QString errorMessage;
#if defined(Q_OS_WIN) && !defined(BUILD_PYTHON_3)
        char * pythonPath;
        static char * initPythonPath();
#endif // Q_OS_WIN && !BUILD_PYTHON_3
};

#endif // PYTHONMODULES_H
