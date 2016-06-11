/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include <Python.h>

#include "pythonmodules.h"
#include "../../version.h"
#include <transformmgmt.h>
#include <QDebug>
#include <QSysInfo>
#include <pip3linecallback.h>

#ifdef BUILD_PYTHON_3
const QString PythonModules::BASE_SCRIPTS_DIR = "python3";
const QString PythonModules::PYTHON_TYPE = "Python 3";
wchar_t PythonModules::PROG_NAME[] = L"pip3line";
#else
const QString PythonModules::BASE_SCRIPTS_DIR = "python27";
const QString PythonModules::PYTHON_TYPE = "Python 2.7";
char PythonModules::PROG_NAME[] = APPNAME;

#endif
const QString PythonModules::PYTHON_EXTENSION = ".py";

const char * PythonModules::MAIN_FUNCTION_NAME = "pip3line_transform";
const char * PythonModules::MODULE_FILE_PATH_STR = "__file__";

// some mandatory initialization on Windows platform (thank you Python dev)
// see Bug https://bugs.python.org/issue26108 -_-'
#if defined(Q_OS_WIN) && !defined(BUILD_PYTHON_3)
#include <windows.h>

char * PythonModules::initPythonPath()
{
    QByteArray newPythonPath;

    // looks for Python libraries path

    QString pythonVersionToSearch;
    pythonVersionToSearch.append(QString::number(PY_MAJOR_VERSION))
            .append(".")
            .append(QString::number(PY_MINOR_VERSION));

    qDebug() << "looking for" << pythonVersionToSearch;

    QStringList pythonRegistry;
    pythonRegistry << QString("HKEY_CURRENT_USER\\SOFTWARE\\Python\\PythonCore\\");
    pythonRegistry << QString("HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\");

    for (int k = 0; k < pythonRegistry.size(); k++) {
        QSettings setting(pythonRegistry.at(k), QSettings::NativeFormat);
        QStringList list = setting.childGroups();
        for (int i = 0; i < list.size(); i++ ) {
            if (pythonVersionToSearch == list.at(i)) {
                setting.beginGroup(list.at(i));
                QStringList groups = setting.childGroups();
                for (int j = 0; j < groups.size(); j++ ) {
                    if (groups.at(j) == QString("InstallPath")) {
                        setting.beginGroup(groups.at(j));
                        newPythonPath.append(setting.value(".").toString().toUtf8());
                        setting.endGroup();
                        break;
                    }
                }
                setting.endGroup();
            }
            if (!newPythonPath.isEmpty())
                break;
        }
    }

    qDebug() << "Found path" << QString::fromUtf8(newPythonPath, newPythonPath.size());

    int baseSize = newPythonPath.size();
    if (baseSize < 1 || baseSize > MAX_PATH) // on Windows MAX_PATH = 260
        qFatal(tr("Something is (once again) wrong with the Python %1 registry key ... can't do anything about it").arg(pythonVersionToSearch).toUtf8().data());

    size_t size = (size_t)baseSize;

    char * pythonHome = new(std::nothrow) char[size + 1];
    memset(pythonHome, 0, size + 1);
    memcpy(pythonHome, newPythonPath.data(), size);

    return pythonHome;
}

#endif //Q_OS_WIN && !BUILD_PYTHON_3

PythonModules::PythonModules(QString name, Pip3lineCallback *callback) :
    ModulesManagement(name, PYTHON_EXTENSION, BASE_SCRIPTS_DIR, callback)
{

    pyGetValFunc = NULL;
    pyStringIO = NULL;
    pyTruncateFunc = NULL;
    pySeekFunc = NULL;

    Py_SetProgramName(PROG_NAME);
    Py_DontWriteBytecodeFlag++;

#if defined(Q_OS_WIN) && !defined(BUILD_PYTHON_3)
    pythonPath = PythonModules::initPythonPath();
    Py_SetPythonHome(pythonPath);
#endif //Q_OS_WIN && !BUILD_PYTHON_3

    // initialize the Python interpreter without signal handler
    Py_InitializeEx(0);
    // initialize thread support
    PyEval_InitThreads();
    // saving thread state
    pymainstate = PyEval_SaveThread();

   connect(this, SIGNAL(pathsUpdated()), SLOT(updatePath()));
   qDebug() << "Created " << this;

}

PythonModules::~PythonModules()
{
    qDebug() << "Destroying " << this;

    unloadModules();

    PyEval_RestoreThread(pymainstate);


    cleaningPyObjs();

    Py_Finalize();

#if defined(Q_OS_WIN) && !defined(BUILD_PYTHON_3)
    // this value is only used for the Python 2.7 module
    delete [] pythonPath;
#endif //Q_OS_WIN && !BUILD_PYTHON_3
}

bool PythonModules::initialize()
{

    settingUpStderr();
    disablingSIGINT();

    // retrieving current sys.path from the Python interpreter
    PyGILState_STATE lgstate;
    lgstate = PyGILState_Ensure();

    QStringList list;
    Py_ssize_t listSize = 0;
    PyObject* sysPath = PySys_GetObject((char*)"path"); // borrowed

    if (PyList_Check(sysPath) != 0) {
        listSize = PyList_Size(sysPath);
        for (Py_ssize_t i = 0; i < listSize; i++) {
            PyObject * pathObj = PyList_GetItem(sysPath,i); // borrowed
            QString pathStr = pyStringToQtString(pathObj);
            if (!pathStr.isEmpty()) {
                list.append(pathStr);
            } else {
                callback->logError(tr("[getCurrentSysPath] Could not convert the pathObj to string"));
                PyGILState_Release(lgstate);
                return false;
            }
        }
    } else {
        callback->logError(tr("[getCurrentSysPath] no sys.path property was found. something is really wrong T_T"));
        PyGILState_Release(lgstate);
        return false;
    }

    PyGILState_Release(lgstate);

    // initializing the parent with the list
    bool ret = ModulesManagement::initialize(list);

    if (ret) updatePath();

    return ret;
}

PyObject *PythonModules::loadModule(QString modulePath, bool reload, bool *firstLoad)
{
    PyObject *pModule = NULL;
    if (modulePath.isEmpty()) { // should have been checked earlier already, but who knows ...
        callback->logError(tr("Empty module path name, nothing to load..."));
        return pModule;
    }

    PyGILState_STATE lgstate;
    lgstate = PyGILState_Ensure();

    if (!modulesPath.contains(modulePath)) {
        qDebug() << "Instanciating the module for the first time " << modulePath;
        QString moduleName = getModuleNameFromFile(modulePath);
        if (!checkModuleNameAndPath(modulePath, moduleName)) { // checking if the module is already there from another file
            PyGILState_Release(lgstate);
            return pModule;
        }
        pModule = PyImport_ImportModule(moduleName.toUtf8().data()); // new reference
        if (!checkPyError()) {
            callback->logError(tr("Module \"%1\" could not be loaded:\n %2").arg(modulePath).arg(errorMessage));
            pModule = NULL;
        } else {
            if (!checkModuleNameAndPath(modulePath, moduleName)) { // checking if the module loaded comes from the file that was supplied
                Py_XDECREF(pModule);
                pModule = NULL;
                PyGILState_Release(lgstate);
                return pModule;
            }

            if (PyObject_HasAttrString(pModule, MAIN_FUNCTION_NAME) != 1) {
                callback->logError(tr("The python module %2 does not have the %1 method").arg(QString::fromUtf8(MAIN_FUNCTION_NAME)).arg(moduleName));
                Py_XDECREF(pModule);
                pModule = NULL;
                PyGILState_Release(lgstate);
                return pModule;
            } else {
                modulesPath.insert(modulePath,pModule);
                if (firstLoad != NULL)
                    *firstLoad = true;
            }
        }

    } else if (reload) {
        qDebug() << "Reloading module" << modulePath;
        PyObject *oldModule = modulesPath.take(modulePath); // the module object is either going to be replaced or cleared
        pModule = PyImport_ReloadModule(oldModule); // new ref ??
        if (pModule != oldModule) {
            Py_XDECREF(oldModule); // clearing the old module object if the new ref is different
        }

        if (!checkPyError()) {
            callback->logError(tr("Error(s) while reloading the module %1, removing it from the the registered modules.\n%2").arg(modulePath).arg(errorMessage));
            pModule = NULL;
        } else {
            modulesPath.insert(modulePath,pModule);
        }

    } else {
        qDebug() << "no reload, taking the module as it is already" << modulePath;
        pModule = modulesPath.value(modulePath);
    }

    PyGILState_Release(lgstate);
    return pModule;
}

bool PythonModules::unloadModules(QString modulePath)
{
    bool ret = false;
    PyGILState_STATE lgstate;
    lgstate = PyGILState_Ensure();
    if (modulesPath.contains(modulePath)) {
        Py_XDECREF(modulesPath.take(modulePath));
        qDebug() << "Module" << modulePath << "removed";
        ret = true;
    }

    PyGILState_Release(lgstate);

    return ret;
}

void PythonModules::unloadModules()
{
    PyGILState_STATE lgstate;
    lgstate = PyGILState_Ensure();
    PyObject * module = NULL;
    Q_FOREACH (module, modulesPath) {
        Py_XDECREF(module);
    }
    modulesPath.clear();

    PyGILState_Release(lgstate);
}

QString PythonModules::getInfos()
{
    QString info;
    info.append(QString("<p>Plugin compiled against Python %1 (%2 bits)</p>").arg(PY_VERSION).arg(QSysInfo::WordSize));
    info.append(QString("<p>Plugin running with %1</p>").arg(getRuntimeVersion()));

    return info;
}

void PythonModules::updatePath()
{
    QStringList list = modulesPaths.keys();
    QString pathString;
#if defined Q_OS_WIN
    char separator  =';';
#else
    char separator  = ':';
#endif


    for (int i = 0; i < list.size(); i++) {
        pathString.append(list.at(i)).append(separator);
    }

    PyGILState_STATE lgstate;
    lgstate = PyGILState_Ensure();

#ifdef BUILD_PYTHON_3
    // Qt garantee that QString.size() is always enough to convert to wchar_t
    int size = pathString.size();
    wchar_t *pathWString = new(std::nothrow) wchar_t[size + 1];
    if (!pathWString)
        qFatal("Cannot allocate memory for pathWString in python 3 plugin");

    int ret = pathString.toWCharArray(pathWString);
    pathWString[size] = L'\x00';
    if (ret != size) {
        callback->logError(tr("conversion to wchar returned %1. expecting %2").arg(ret).arg(size));
    }

    PySys_SetPath(pathWString);

    delete [] pathWString;
#else
    PySys_SetPath(pathString.toUtf8().data());
#endif

    PyGILState_Release(lgstate);
}

void PythonModules::settingUpStderr()
{
    PyGILState_STATE lgstate;
    lgstate = PyGILState_Ensure();

    PyObject *modStringIO = NULL;
    PyObject *obFuncStringIO = NULL;
    char stderrString[] = "stderr";

    // Import cStringIO module
#ifdef BUILD_PYTHON_3
    modStringIO = PyImport_ImportModule("io");
#else
    modStringIO = PyImport_ImportModule("cStringIO");
#endif
    if (!checkPyObject(modStringIO)){
#ifdef BUILD_PYTHON_3
        callback->logError("[stderr init]Importing io failed");
#else
        callback->logError("[stderr init]Importing cStringIO failed");
#endif
        Py_XDECREF(modStringIO);
        PyGILState_Release(lgstate);
        return;
    }
    // get StringIO constructor
    obFuncStringIO = PyObject_GetAttrString(modStringIO, "StringIO");
    if (!checkPyObject(obFuncStringIO)){
        callback->logError("[stderr init] can't find io.StringIO");
        Py_XDECREF(modStringIO);
        Py_XDECREF(obFuncStringIO);
        PyGILState_Release(lgstate);
        return;
    }
    // Construct cStringIO object
    pyStringIO = PyObject_CallObject(obFuncStringIO, NULL);
    if (!checkPyObject(pyStringIO)) {
        callback->logError("[stderr init] StringIO() failed");
        Py_XDECREF(modStringIO);
        Py_XDECREF(obFuncStringIO);
        cleaningPyObjs();
        PyGILState_Release(lgstate);
        return;
    }
    Py_XDECREF(obFuncStringIO);
    Py_XDECREF(modStringIO);

    // create getvalue() callable from StringIO
    pyGetValFunc = PyObject_GetAttrString(pyStringIO, "getvalue");
    if (!checkPyObject(pyGetValFunc)){
        callback->logError("[stderr init] can't find getvalue() function");
        cleaningPyObjs();
        PyGILState_Release(lgstate);
        return;
    }

    // create truncate() callable from StringIO
    pyTruncateFunc = PyObject_GetAttrString(pyStringIO, "truncate");
    if (!checkPyObject(pyTruncateFunc)){
        callback->logError("[stderr init] can't find truncate() function");
        cleaningPyObjs();
        PyGILState_Release(lgstate);
        return;
    }

    // create seek() callable from StringIO
    pySeekFunc = PyObject_GetAttrString(pyStringIO, "seek");
    if (!checkPyObject(pySeekFunc)){
        callback->logError("[stderr init] can't find seek() function");
        cleaningPyObjs();
        PyGILState_Release(lgstate);
        return;
    }

    // try assigning this object to sys.stderr
    int ret = PySys_SetObject(stderrString, pyStringIO);
    if (ret != 0) {
        callback->logError("[stderr init] failed to assign io.StringIO object to stderr");
        PyErr_Print();
        cleaningPyObjs();
    }

    PyGILState_Release(lgstate);
}

void PythonModules::disablingSIGINT()
{
    PyGILState_STATE lgstate;
    lgstate = PyGILState_Ensure();

    PyObject *modSignals = NULL;
    // Import signal module

    modSignals = PyImport_ImportModule("signal");

    if (!checkPyObject(modSignals)){
        callback->logError("[disablingSIGINT] Importing signal failed, Python SIGINT handler is NOT disabled");
        Py_XDECREF(modSignals);
        PyGILState_Release(lgstate);
        return;
    }

    // get signal.SIGINT
    PyObject *pySIGINT = PyObject_GetAttrString(modSignals, "SIGINT");
    if (!checkPyObject(pySIGINT)){
        callback->logError("[disablingSIGINT] Failed to retrieve signal.SIGINT");
        Py_XDECREF(modSignals);
        PyGILState_Release(lgstate);
        return;
    }

    // get signal.SIG_IGN (the handler that ignore a signal)
    PyObject *pySIG_IGN = PyObject_GetAttrString(modSignals, "SIG_IGN");
    if (!checkPyObject(pySIG_IGN)){
        callback->logError("[disablingSIGINT] Failed to retrieve signal.SIG_IGN");
        Py_XDECREF(modSignals);
        Py_XDECREF(pySIGINT);
        PyGILState_Release(lgstate);
        return;
    }

    // create signal() callable from signal
    PyObject * pySignalFunc = PyObject_GetAttrString(modSignals, "signal");
    if (!checkPyObject(pySignalFunc)){
        checkPyError();
        {
            callback->logError(tr("%1").arg(errorMessage));
        }
        callback->logError("[disablingSIGINT] Failed to retrieve signal.signal()");
        Py_XDECREF(modSignals);
        Py_XDECREF(pySIGINT);
        Py_XDECREF(pySIG_IGN);
        PyGILState_Release(lgstate);
        return;
    }

    PyObject * pArgs = PyTuple_New(2); // new ref for arguments
    if (!checkPyObject(pArgs)){
        callback->logError("[disablingSIGINT] Failed to create argument tuple");
        Py_XDECREF(modSignals);
        Py_XDECREF(pySIGINT);
        Py_XDECREF(pySIG_IGN);
        Py_XDECREF(pySignalFunc);
        PyGILState_Release(lgstate);
        return;
    }


    if (PyTuple_SetItem(pArgs, 0, pySIGINT) != 0) { // don't need to clean pySIGINT at this point (stolen)
        callback->logError("[disablingSIGINT] Error while assigning the SIGINT value to the arg tuple");
        Py_XDECREF(modSignals);
        Py_XDECREF(pySIGINT);
        Py_XDECREF(pySIG_IGN);
        Py_XDECREF(pySignalFunc);
        Py_XDECREF(pArgs);
        PyGILState_Release(lgstate);
        return;
    }

    if (PyTuple_SetItem(pArgs, 1, pySIG_IGN) != 0) { // don't need to clean pySIG_IGN at this point (stolen)
        callback->logError("[disablingSIGINT] Error while assigning the SIG_IGN value to the arg tuple");
        Py_XDECREF(modSignals);
        Py_XDECREF(pySIG_IGN);
        Py_XDECREF(pySignalFunc);
        Py_XDECREF(pArgs);
        PyGILState_Release(lgstate);
        return;
    }

    PyObject * obResult = PyObject_CallObject(pySignalFunc, pArgs); // new ref or NULL
    if (!checkPyError()){
        callback->logError("[disablingSIGINT] signal.signal(SIGINT, SIG_IGN) failed");
        callback->logError(tr("%1").arg(errorMessage));
    }

    qDebug() << "Python SIGINT handler disabled";
    Py_XDECREF(obResult);
    Py_XDECREF(pArgs);
    Py_XDECREF(modSignals);

    PyGILState_Release(lgstate);
}

bool PythonModules::checkModuleNameAndPath(QString modulePath, QString moduleName)
{
    PyGILState_STATE lgstate;
    lgstate = PyGILState_Ensure();
    bool ret = false;

    PyObject* pymodules = PySys_GetObject((char*)"modules"); // borrowed

    if (PyDict_Check(pymodules) != 0) {
        PyObject* moduleChecked = PyUnicode_FromStringAndSize(moduleName.toUtf8(),moduleName.toUtf8().size());
        if (!checkPyError()) {
            qCritical() << tr("Error while creating Python current module key string T_T :\n%1").arg(getLastError());
        } else if (PyDict_Contains(pymodules, moduleChecked)  == 1) {
            PyObject * loadedModule = PyDict_GetItem(pymodules, moduleChecked); //borrowed
            if (loadedModule != NULL) {
                if (PyObject_HasAttrString(loadedModule, MODULE_FILE_PATH_STR) == 1) {
                    PyObject * loadedModulePath = PyObject_GetAttrString(loadedModule, MODULE_FILE_PATH_STR); // new ref
                    if (loadedModulePath != NULL) {
                        QString modPathString = pyStringToQtString(loadedModulePath);
                        if (modPathString == modulePath) {
                            ret = true;
                        } else {
                            QString message = tr("Another module with the same name (%1) has been loaded from %2").arg(moduleName).arg(modPathString);
                            callback->logError(message);
                        }
                        Py_XDECREF(loadedModulePath);
                    } else {
                        qCritical() << tr("NULL object retrieved while getting the module file name for %1 T_T").arg(moduleName);
                    }
                } else {
                    qCritical() << tr("module object %1 does not have the %2 attribute T_T").arg(moduleName).arg(QString::fromUtf8(MODULE_FILE_PATH_STR));
                }
            } else {
                qCritical() << tr("NULL object retrieved while getting the module for %1 T_T").arg(moduleName);
            }
        } else {
            ret = true;
        }
        Py_XDECREF(moduleChecked);
    } else {
        qCritical() <<  tr("sys.modules is not a dictionary T_T");
    }

    PyGILState_Release(lgstate);

    return ret;
}

bool PythonModules::checkPyError()
{
    if (PyErr_Occurred()) {
        errorMessage = retrievePythonErrors();
        return false;
    }
    errorMessage.clear();
    return true;
}

QString PythonModules::getLastError()
{
    return errorMessage;
}

QString PythonModules::retrievePythonErrors()
{
    QString message;
    QString messageStr;
    PyObject* pArgs = NULL;
    PyObject* inputPy = NULL;
    QString final = "[Script error]";
    PyObject *obResult = NULL;
    PyGILState_STATE lgstate;
    lgstate = PyGILState_Ensure();

    PyErr_Print(); // Dump the error message(s) in the buffer
    if (pyGetValFunc == NULL || pyTruncateFunc == NULL || pySeekFunc ==NULL || pyStringIO == NULL) {
        callback->logError(tr("The error catching mecanism was not properly initialized, ignoring Python error request."));
        goto leaving;
    }

    // call getvalue() method in StringIO instance

    obResult = PyObject_CallObject(pyGetValFunc, NULL); // new ref or NULL
    if (!checkPyObject(obResult)){
        callback->logError("[stderr read] getvalue() failed");
        Py_XDECREF(obResult);
        goto leaving;
    }

    messageStr = pyStringToQtString(obResult);
    if (!messageStr.isEmpty()) {
        final.append(messageStr);
    }

    if (final.endsWith('\n')) {
        final.chop(1);
    }
    message = final;

    Py_XDECREF(obResult);
    obResult = NULL;

    // Cleaning the StringIO object
    pArgs = PyTuple_New(1); // new ref

    if (!checkPyObject( pArgs)) {
        callback->logError("[stderr cleaning] Error while creating the Python argument tuple");
        Py_XDECREF(pArgs);
        goto leaving;
    }

    inputPy = PyLong_FromLong(0); // new ref
    if (!checkPyObject( inputPy)) {
        callback->logError("[stderr cleaning] Error while creating the Python int value (0)");
        Py_XDECREF(pArgs);
        Py_XDECREF(inputPy);
        goto leaving;
    }

    if (PyTuple_SetItem(pArgs, 0, inputPy) != 0) { // don't need to clean inputPy at this point (stolen)
        callback->logError("[stderr cleaning] Error while assigning the long value to the arg tuple");
        Py_XDECREF(pArgs);
        Py_XDECREF(inputPy);
        goto leaving;
    }

    obResult = PyObject_CallObject(pyTruncateFunc, pArgs); // new ref or NULL
    if (!checkPyObject(obResult)){
        callback->logError("[stderr cleaning] truncate() failed");
    } else {
        Py_XDECREF(obResult);
    }
    obResult = NULL;
    Py_XDECREF(pArgs);

    // seek(0)

    pArgs = PyTuple_New(1); // new ref

    if (!checkPyObject( pArgs)) {
        callback->logError("[stderr cleaning] Error while creating the Python argument tuple");
        Py_XDECREF(pArgs);
        goto leaving;
    }

    inputPy = PyLong_FromLong(0); // new ref
    if (!checkPyObject( inputPy)) {
        callback->logError("[stderr cleaning] Error while creating the Python int value (0)");
        Py_XDECREF(pArgs);
        Py_XDECREF(inputPy);
        goto leaving;
    }

    if (PyTuple_SetItem(pArgs, 0, inputPy) != 0) { // don't need to clean inputPy at this point (stolen)
        Py_XDECREF(pArgs);
        Py_XDECREF(inputPy);
        goto leaving;
    }

    obResult = PyObject_CallObject(pySeekFunc, pArgs); // new ref or NULL
    if (!checkPyObject(obResult)){
        callback->logError("[stderr cleaning] seek() failed");
    } else {
        Py_XDECREF(obResult);
    }

    Py_XDECREF(pArgs);
leaving:
    PyGILState_Release(lgstate);

    return message;
}

void PythonModules::cleaningPyObjs()
{
    PyGILState_STATE lgstate;
    lgstate = PyGILState_Ensure();
    Py_XDECREF(pyTruncateFunc);
    pyTruncateFunc = NULL;
    Py_XDECREF(pyGetValFunc);
    pyGetValFunc = NULL;
    Py_XDECREF(pySeekFunc);
    pySeekFunc = NULL;
    Py_XDECREF(pyStringIO);
    pyStringIO = NULL;
    PyGILState_Release(lgstate);
}

QString PythonModules::getRuntimeVersion()
{
    QString version;
    PyGILState_STATE lgstate;
    lgstate = PyGILState_Ensure();

    PyObject *modSys = NULL;
    // Import signal module

    modSys = PyImport_ImportModule("sys");

    if (!checkPyObject(modSys)){
        qCritical() << tr("[PythonModules::getRuntimeVersion] Importing sys failed");
    } else {
        // get sys.version
        PyObject *pyVersion = PyObject_GetAttrString(modSys, "version");
        if (!checkPyObject(pyVersion)){
            qCritical() <<  tr("[PythonModules::getRuntimeVersion] Failed to retrieve sys.version");
        } else {
            version = pyStringToQtString(pyVersion);
            if (version.isEmpty()) {
                callback->logError(tr("Failed to retrieve the Python runtime version String"));
            }
        }

        Py_XDECREF(pyVersion);
    }
    Py_XDECREF(modSys);
    PyGILState_Release(lgstate);

    return version;
}

QString PythonModules::pyStringToQtString(PyObject *strPyObj)
{
    QString ret;
    Py_ssize_t size = 0;
#ifdef BUILD_PYTHON_3
    if (PyUnicode_Check(strPyObj)) {
        wchar_t *wstring = PyUnicode_AsWideCharString(strPyObj, &size); // new object, need to be cleaned
        if (wstring != NULL) {
            ret = QString::fromWCharArray(wstring,size);
            PyMem_Free(wstring);
#else
    if (PyString_Check(strPyObj)) {
        char * buf = NULL;
        if (PyString_AsStringAndSize(strPyObj, &buf, &size) != -1) { // do not touch buf after the call
            ret = QString::fromUtf8(QByteArray(buf, size));
#endif
        } else {
            qCritical() << tr("[PythonModules::pyStringToQtString] Could not convert the PyObject to string");
        }
    } else {
        qCritical() << tr("[PythonModules::pyStringToQtString] the given object is not a string");
    }
    return ret;
}

bool PythonModules::checkPyObject(PyObject *obj)
{
    return !(PyErr_Occurred() || obj == NULL);
}
