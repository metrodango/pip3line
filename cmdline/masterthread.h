/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef MASTERTHREAD_H
#define MASTERTHREAD_H

#include <QThread>
#include <QString>
#include <QTextStream>
#include <Qt>
#include <transformmgmt.h>
#include "../tools/processor.h"

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
#define QTENDL Qt::endl
#else
#define QTENDL endl
#endif

class MasterThread : public QThread
{
        Q_OBJECT
    public:
        explicit MasterThread(bool binaryInput, bool hideErrorsFlag, bool verboseFlag, QObject *parent = 0);
        ~MasterThread();
        void run();
        void setConfigurationFile(const QString &fileName, bool inbound = true);
        void setTransformName(const QString &transformName, bool inbound = true);
        
    signals:
        
    public slots:
        void logError(const QString mess, const QString &source = QString());
        void logStatus(const QString mess, const QString &source = QString());
    private:
        QTextStream *errlog;
        QTextStream *messlog;
        bool verbose;
        bool hideErrors;
        bool binaryInput;
        QString confFileName;
        QString transformName;
        TransformMgmt transformFactory;
        TransformAbstract::Way singleWay;
        Processor *processor;
};

#endif // MASTERTHREAD_H
