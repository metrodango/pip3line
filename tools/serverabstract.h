/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef SERVERABSTRACT_H
#define SERVERABSTRACT_H

#include <QMutex>
#include <QObject>
#include <transformmgmt.h>
#include "processor.h"
#include "processingstats.h"

class ProcessingStats;

class ServerAbstract : public QObject
{
        Q_OBJECT
    public:
        explicit ServerAbstract(TransformMgmt *tFactory);
        virtual ~ServerAbstract();
        virtual void setOutput(QIODevice *out);
        void clearOutput();
        ProcessingStats getStats();

        virtual void stopServer();
        virtual bool startServer() = 0;
        virtual QString getLastError() = 0;
        virtual QString getServerType() = 0;
        bool getAllowForwardingLogs() const;
        void setAllowForwardingLogs(bool value);

    public slots:
        void setEncoding(bool flag);
        void setDecoding(bool flag);
        void setTransformations(const QString &conf);
        void setSeparator(char c);
    signals:
        void error(const QString message, const QString src);
        void status(const QString message, const QString src);
        void newTransformChain(QString chainConf);
    protected slots:
        virtual void logError(const QString mess, const QString id);
        virtual void logStatus(const QString mess, const QString id);
    protected:
        QIODevice * output;
        QMutex confLocker;
        TransformMgmt * transformFactory;
        QString tconf;
        char separator;
        bool encode;
        bool decode;
        QList<Processor *> clientProcessor;
        ProcessingStats stats;
        bool allowForwardingLogs;
    private:
        Q_DISABLE_COPY(ServerAbstract)
};

#endif // SERVERABSTRACT_H
