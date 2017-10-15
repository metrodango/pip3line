/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef PYTHONTRANSFORM_H
#define PYTHONTRANSFORM_H

#include <scripttransformabstract.h>

class PythonModules;
typedef struct _object PyObject;

class PythonTransform : public ScriptTransformAbstract
{
        Q_OBJECT
    public:
        static const QString id;
        explicit PythonTransform(PythonModules * mmanagement, const QString &name = QString());
        ~PythonTransform();

        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        QString help() const;
        bool setModuleFile(const QString &fileName);
        QString inboundString() const;
        QString outboundString() const;
    public Q_SLOTS:
        bool loadModule();
    private:
        Q_DISABLE_COPY(PythonTransform)
        bool loadModuleAttributes();
        PyObject * pModule;
        bool twoWays;
        PythonModules * pythonmgm;
};

#endif // PYTHONTRANSFORM_H
