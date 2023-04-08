/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <QObject>
#include <QThread>
#include <QTextStream>
#include <QList>
#include <QMutex>
#include <transformchain.h>
#include <transformabstract.h>
#include "processingstats.h"

class TransformMgmt;
class QIODevice;

class Processor : public QThread
{
        Q_OBJECT
    public:
        static const char DEFAULT_SEPARATOR;
        explicit Processor(TransformMgmt * transformFactory, QObject * parent);
        virtual ~Processor();
        bool configureFromFile(const QString &fileName);
        bool configureFromName(const QString &name, TransformAbstract::Way way = TransformAbstract::INBOUND);
        bool setTransformsChain(TransformChain tlist); // will dispose of the list itself

        void setOutputMutex(QMutex * mutex);
        void setOutput(QIODevice * nout);
        void setInput(QIODevice * nin);
        void clearOutputMutex();
        void setSeparator(char c);
        ProcessingStats getStats();
        void setEncoding(bool flag);
        void setDecoding(bool flag);
        virtual void stop() {}
    public slots:
        bool setTransformsChain(const QString &xmlConf);
    signals:
        void error(const QString, const QString);
        void status(const QString, const QString);

    protected slots:
        void logError(const QString mess, const QString id);
        void logStatus(const QString mess, const QString id);
    protected:
        Processor() {}
        virtual void writeBlock(const QByteArray &data);
        TransformChain tlist;
        QMutex * outputLock;
        QIODevice *out;
        QIODevice *in;
        char separator;
        ProcessingStats stats;
    private:
        Q_DISABLE_COPY(Processor)
        inline void clearChain();
        TransformMgmt * transformFactory;
        QMutex tranformsLock;
        QMutex statsLock;
        bool encode;
        bool decode;

};

#endif // PROCESSOR_H
