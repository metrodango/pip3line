/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include <Python.h>
#include "pythonmodules.h"
#include <QHashIterator>
#include <QDebug>
#include <QCoreApplication>
#include <QThread>

#include "pythontransform.h"

#ifdef BUILD_PYTHON_3
const QString PythonTransform::id = "Python 3 script";
#else
const QString PythonTransform::id = "Python 2.7 script";
#endif

PythonTransform::PythonTransform(PythonModules * mmanagement, const QString &name) :
    ScriptTransformAbstract(mmanagement, name)
{
    pModule = nullptr;
    twoWays = false;
    pythonmgm = mmanagement;
}

PythonTransform::~PythonTransform()
{
//    qDebug() << "Destroying " << this;
}

QString PythonTransform::description() const
{
    return id;
}

void PythonTransform::transform(const QByteArray &input, QByteArray &output)
{
    if (input.isEmpty())
        return;

    PyGILState_STATE lgstate;
    lgstate = PyGILState_Ensure();
    if (loadModule()) {
        PyObject * pyInbound = Py_False; // needs reference count management

        if (twoWays)
            pyInbound = (wayValue == INBOUND ? Py_True : Py_False );
        Py_INCREF(pyInbound);

        if (PyModule_AddObject(pModule, PythonModules::INBOUND_ATTR_NAME, pyInbound) == -1) { // steal reference
            pythonmgm->checkPyError();
            logError(tr("T_T Could not set the direction value properly:\n%1").arg(pythonmgm->getLastError()),id);
            Py_XDECREF(pyInbound);
            PyGILState_Release(lgstate);
            return;
        }

        PyObject *paramsdict = PyDict_New(); // setting an empty dictionary
        // setting parameters in the python environment
        if (!parameters.isEmpty()) {
            if (!pythonmgm->checkPyError()) {
                logError(tr("T_T Error while creating the Python parameter dict:\n%1").arg(pythonmgm->getLastError()), id);
                Py_XDECREF(paramsdict);
                PyGILState_Release(lgstate);
                return;
            }
            // adding parameters to the python list
            QHashIterator<QByteArray, QByteArray> i(parameters);
            while (i.hasNext()) {
                i.next();
                PyObject* paramKey = PyUnicode_FromStringAndSize(i.key(),i.key().size());
                if (!pythonmgm->checkPyError()) {
                    logError(tr("T_T Error while creating Python parameter key:\n%1").arg(pythonmgm->getLastError()), id);
                    Py_XDECREF(paramsdict);
                    PyGILState_Release(lgstate);
                    return;
                }

                PyObject* paramValue = PyUnicode_FromStringAndSize(i.value(),i.value().size());
                if (!pythonmgm->checkPyError()) {
                    logError(tr("T_T Error while creating Python parameter value:\n%1").arg(pythonmgm->getLastError()), id);
                    Py_XDECREF(paramsdict);
                    Py_XDECREF(paramKey);
                    PyGILState_Release(lgstate);
                    return;
                }

                if (PyDict_SetItem(paramsdict,paramKey,paramValue) == -1) { // not stealing reference
                    pythonmgm->checkPyError(); // we already know there was an error
                    logError(tr("T_T Error while setting Python parameter pair:\n%1").arg(pythonmgm->getLastError()), id);
                    Py_XDECREF(paramsdict);
                    Py_XDECREF(paramKey);
                    Py_XDECREF(paramValue);
                    PyGILState_Release(lgstate);
                    return;
                }

                // Cleaning the values (references not stolen)

                Py_XDECREF(paramKey);
                Py_XDECREF(paramValue);
            }
        }

        // setting the dictionary in any case, even if it is empty
        if (PyModule_AddObject(pModule,PythonModules::PARAMS_ATTR_NAME , paramsdict) == -1) { // stolen paramsdict reference
            pythonmgm->checkPyError();
            logError(tr("T_T Could not set the Pip3line_params value properly:\n%1").arg(pythonmgm->getLastError()),id);
        }

        PyObject * pFunc = PyObject_GetAttrString(pModule, PythonModules::MAIN_FUNCTION_NAME);

        if (pythonmgm->checkPyError() && PyCallable_Check(pFunc)) {
            PyObject* pArgs = PyTuple_New(1);

            if (!pythonmgm->checkPyError()) {
                Q_EMIT error(tr("T_T Error while creating the Python argument tuple:\n%1").arg(pythonmgm->getLastError()), id);
                Py_XDECREF(pFunc);
                Py_XDECREF(pArgs);
                PyGILState_Release(lgstate);
                return;
            }

            PyObject* inputPy = PyByteArray_FromStringAndSize(input.data(),input.size());
            if (!pythonmgm->checkPyError()) {
                Q_EMIT error(tr("T_T Error while creating the Python byte array:\n%1").arg(pythonmgm->getLastError()), id);
                Py_XDECREF(pFunc);
                Py_XDECREF(pArgs);
                Py_XDECREF(inputPy);
                PyGILState_Release(lgstate);
                return;
            }

            if (PyTuple_SetItem(pArgs, 0, inputPy) != 0) {// stealing the reference of inputPy
                pythonmgm->checkPyError();
                Q_EMIT error(tr("T_T Error while creating the Python byte array:\n%1").arg(pythonmgm->getLastError()), id);
                Py_XDECREF(inputPy);
                Py_XDECREF(pFunc);
                Py_XDECREF(pArgs);
                PyGILState_Release(lgstate);
                return;
            }
            PyObject* returnValue = PyObject_CallObject(pFunc, pArgs); // new ref or NULL

            if (!pythonmgm->checkPyError()) {
                Q_EMIT error(tr("T_T Python error while executing the function:\n %1").arg(pythonmgm->getLastError()), id);
            } else {
                if (PyByteArray_Check(returnValue)) {

                    Py_ssize_t templength = PyByteArray_Size(returnValue);
                    if (templength > BLOCK_MAX_SIZE) {
                        templength = BLOCK_MAX_SIZE;
                        Q_EMIT warning(tr("Data block returned is too large, truncating."),id);
                    }

                    char * buffer = PyByteArray_AsString(returnValue); // never to be deleted
                    output.append(QByteArray(buffer,static_cast<int>(templength))); // safe cast as value was checked earlier
                } else {
                    Q_EMIT error(tr("The Python object returned is not a bytearray"), id);
                }
            }

            Py_XDECREF(returnValue);
            Py_XDECREF(pArgs);
          //  Py_DECREF(inputPy); // stolen reference, don't touch that
            Py_XDECREF(pFunc);

        } else {
            Q_EMIT error(tr("Python error while calling the function %1():\n%2").arg(PythonModules::MAIN_FUNCTION_NAME).arg(pythonmgm->getLastError()), id);
        }
    } else {
        qDebug() << "[Python transform] could not load the module";
    }

    PyGILState_Release(lgstate);
}

bool PythonTransform::isTwoWays()
{
    return twoWays;
}

QString PythonTransform::help() const
{
    QString help;
    help.append(tr("<p>%1</p>").arg(id));
    help.append(tr("<p>The list of Python transforms is loaded automatically from differents default locations.</p><p>In addition modules are reloaded (refreshed) every time the input is refreshed, so you don't need to restart Pip3line to commit scripts changes</p><p>Using Python %1</p><p>Current Python module name: \"%2\"</p>").arg(QString(Py_GetVersion())).arg(moduleName));
    return help;
}

bool PythonTransform::setModuleFile(const QString &fileName)
{
    if (type == ModulesManagement::AUTO) {
        Q_EMIT error(tr("Not allowed to change the module for auto loaded modules"),id);
        return false;
    }
    if (fileName == moduleFileName) {
        qDebug() << "nothing to be done here, filename already set";
        return true; // nothing to be done here
    }

    QString val = moduleManagement->addModule(fileName, type);
    if (!val.isEmpty()) {
        PyGILState_STATE lgstate;
        lgstate = PyGILState_Ensure();
        Py_XDECREF(pModule);
        pModule = nullptr;
        moduleName = val;
        moduleFileName = fileName;

        if  (!loadModule()) {
           Q_EMIT error(tr("Error while loading module"),id);
        } else {
            qDebug() << "Module appeared to have been loaded successfully";
        }
        PyGILState_Release(lgstate);
        return true;
    }
    return false;
}

QString PythonTransform::inboundString() const
{
    return "Inbound";
}

QString PythonTransform::outboundString() const
{
    return "Outbound";
}

bool PythonTransform::loadModuleAttributes()
{
    if (pModule == nullptr) {
        Q_EMIT error(tr("The module object is NULL for %1, could not (re)load the configuration").arg(moduleFileName),id);
        return false;
    }

    qDebug() << "Loading module attributes" << moduleName;

    bool ret = true;
    bool oldtwoWays = twoWays;
    twoWays = false; // setting default

    PyGILState_STATE lgstate;
    lgstate = PyGILState_Ensure();

    // checking if the two ways attribute is there
    PyObject * twoWayAttr = PyUnicode_FromString(PythonModules::ISTWOWAY_ATTR_NAME); // New ref
    if (pythonmgm->checkPyError()) {
        if (PyObject_HasAttr(pModule,twoWayAttr) == 1) {  // does the module has the attribute?
            PyObject * pyTwoWay = PyObject_GetAttr(pModule,twoWayAttr); // New ref
            if (pythonmgm->checkPyError()) {
                twoWays =  pyTwoWay == Py_True;
            } else {
                logError(tr("T_T Error while getting attribute value ISTWOWAY_ATTR_NAME for %1:\n%2").arg(moduleFileName).arg(pythonmgm->getLastError()),id);
            }
            Py_XDECREF(pyTwoWay);
        } else {
            qDebug() << moduleFileName << "has no attribute" << PythonModules::ISTWOWAY_ATTR_NAME;
        }
    } else {
        logError(tr("T_T Error while converting to Unicode string:\n%1").arg(pythonmgm->getLastError()),id);
    }
    Py_XDECREF(twoWayAttr);

    bool parametersChanged = false;
    // checking if some default parameters names were defined
    PyObject * paramsNamesAttr = PyUnicode_FromString(PythonModules::PARAMS_NAMES_ATTR_NAME); // New ref
    if (pythonmgm->checkPyError()) {
        if (PyObject_HasAttr(pModule,paramsNamesAttr) == 1) { // does the module has the attribute?
            PyObject * pyNamesList = PyObject_GetAttr(pModule,paramsNamesAttr); // New ref
            if (pythonmgm->checkPyError()) {
                if (PyList_Check(pyNamesList)) {
                    Py_ssize_t listSize = PyList_Size(pyNamesList);
                    if (listSize > 0) { // if list size is null then nothing to do
                        for (int i = 0; i < listSize; i++) {
                            QByteArray val;
                            PyObject *pyName = PyList_GetItem(pyNamesList, i); // borrowed ref
                            if (pythonmgm->checkPyError()) { // error or invalid?
#ifdef BUILD_PYTHON_3
                                if (PyUnicode_Check(pyName)) { // is this a unicode string?
                                    PyObject * nameutf8 = PyUnicode_AsUTF8String(pyName); // new ref
                                    if (pythonmgm->checkPyError() && nameutf8 != nullptr) {
                                        val = QByteArray(PyBytes_AsString(nameutf8), PyBytes_Size(nameutf8));
                                    } else {
                                        logError(tr("Error while encoding a parameter to UTF-8:%1").arg(pythonmgm->getLastError()),id);
                                    }
                                    Py_XDECREF(nameutf8);
#else
                                if (PyString_Check(pyName)) { // is this a string?
                                    val = QByteArray(PyString_AsString(pyName), PyString_Size(pyName));
#endif
                                    if (val.isEmpty()) { // if the parameter name is empty, we skip
                                        logWarning(tr("The Python object %1[%2] is an empty string, ignoring.").arg(PythonModules::PARAMS_NAMES_ATTR_NAME).arg(i),id);
                                    } else if (!parameters.contains(val)) { // we don't want to erase any pre-existing configuration
                                        parameters.insert(val, QByteArray());
                                        parametersChanged = true;
                                    }
                                } else {
                                    logWarning(tr("The Python object %1[%2] is not a string, ignoring.").arg(PythonModules::PARAMS_NAMES_ATTR_NAME).arg(i),id);
                                }
                            } else {
                                logError(tr("T_T Error while getting the item from attribute list:\n%1").arg(pythonmgm->getLastError()),id);
                            }
                        }
                    } else {
                        logWarning(tr("The Python object for attribute names (%1) is empty, ignoring.").arg(PythonModules::PARAMS_NAMES_ATTR_NAME),id);
                    }
                } else {
                    logWarning(tr("The Python object for attribute names (%1) is not a list, ignoring.").arg(PythonModules::PARAMS_NAMES_ATTR_NAME),id);
                }

            } else {
                logError(tr("T_T Error while getting the attribute %1:\n%2").arg(PythonModules::PARAMS_NAMES_ATTR_NAME).arg(pythonmgm->getLastError()),id);
            }
            Py_XDECREF(pyNamesList);
            pyNamesList = nullptr;
        } else {
            qDebug() << moduleFileName << " does not have attribute" << PythonModules::PARAMS_NAMES_ATTR_NAME;
        }
    } else {
        logError(tr("T_T Error while converting to Unicode string:\n%1").arg(pythonmgm->getLastError()),id);
    }
    Py_XDECREF(paramsNamesAttr);

    if (oldtwoWays != twoWays || parametersChanged) { // notifying only if something has changed
        Q_EMIT confUpdated();
    }


    PyGILState_Release(lgstate);
    return ret;
}

bool PythonTransform::loadModule()
{
    if (moduleFileName.isEmpty()) {
        Q_EMIT error(tr("The module file name is empty, skipping import"),id);
        return false;
    }

    PythonModules * pythonManager = static_cast<PythonModules *>(moduleManagement);
    bool firstLoad = false;
    pModule = pythonManager->loadModule(moduleFileName,autoReload, &firstLoad);

    if (pModule != nullptr) {
        if (autoReload || firstLoad) {
            loadModuleAttributes();
        }
    } else {
        qDebug() << "pModule is NULL";
    }

    return pModule != nullptr; // Was there an error during loading
}

