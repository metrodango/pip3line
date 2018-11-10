/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef PIP3LINECALLBACK_H
#define PIP3LINECALLBACK_H

#include <QObject>
#include <QSettings>
#include "libtransform_global.h"

class TransformMgmt;
class TransformAbstract;
class QXmlStreamReader;

class LIBTRANSFORMSHARED_EXPORT Pip3lineCallback : public QObject
{
        Q_OBJECT
        friend class TransformMgmt;
    public:
        void saveParameter(const QString &name, const QVariant &value);
        QVariant getParameter(const QString &name);
        QStringList pip3lineDirs();
        TransformAbstract *getTransformFromXML(QXmlStreamReader *xmlConf);
    Q_SIGNALS:
        void error(const QString, const QString);
        void warning(const QString, const QString);
        void status(const QString, const QString);
        void newTransform();
        
    public Q_SLOTS:
        void logError(const QString &message);
        void logWarning(const QString &message);
        void logStatus(const QString &message);
        void notifyNewTransform();
    private:
        Q_DISABLE_COPY(Pip3lineCallback)
        explicit Pip3lineCallback() {}
        explicit Pip3lineCallback(TransformMgmt *tfactory, const QString &confFile, const QString &pluginName, QObject *parent = nullptr);
        QString pluginName;
        QSettings settings;
        TransformMgmt *transformFactory;
        
};

#endif // PIP3LINECALLBACK_H
