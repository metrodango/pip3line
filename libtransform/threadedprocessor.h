/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#ifndef THREADEDPROCESSOR_H
#define THREADEDPROCESSOR_H

#include <QObject>
#include <QThread>
#include <QFuture>
#include <QQueue>
#include <QHash>
#include "libtransform_global.h"
#include "transformabstract.h"

class LIBTRANSFORMSHARED_EXPORT TransformRequest : public QObject
{
        Q_OBJECT
     public:
        explicit TransformRequest(TransformAbstract *transform, const QByteArray &in, quintptr nptid,bool takeOwnerShip = true);
        explicit TransformRequest(TransformAbstract *transform, const QList<QByteArray> &ins, quintptr nptid,bool takeOwnerShip = true);
        ~TransformRequest();
        void runRequest();
        quintptr getptid() const;
    signals:
        void finishedProcessing(QByteArray output, Messages messages);
        void finishedProcessing(QList<QByteArray> outputs, Messages messages);
        void endOfProcess();
    private slots:
        void logError(QString message, QString source = QString());
        void logWarning(QString message, QString source = QString());
        void logStatus(QString message, QString source = QString());
    private:
        Q_DISABLE_COPY(TransformRequest)
        void logMessage(QString message, QString source = QString(), LOGLEVEL level = PLSTATUS);
        const QByteArray inputData;
        const QList<QByteArray> inputDatas;
        QByteArray outputData;
        QList<QByteArray> outputDatas;
        TransformAbstract *transform;
        QList<Message> messagesList;
        quintptr ptid;
        bool deleteObject;
        bool multiProcessing;
};


class LIBTRANSFORMSHARED_EXPORT ThreadedProcessor : public QObject
{
        Q_OBJECT
    public:
        explicit ThreadedProcessor(QObject *parent = 0);
        ~ThreadedProcessor();
    public slots:
        void processRequest(TransformRequest * request);
    private slots:
        void onRequestFinished();
    private:
        Q_DISABLE_COPY(ThreadedProcessor)
        void startRequest(TransformRequest * request);
        bool isSourceRunning(quintptr source);
        QHash<TransformRequest *, QFuture<void> > currentRunning;
        QHash<quintptr, TransformRequest *> waitingRequests;
        QThread workerThread;
};

#endif // THREADEDPROCESSOR_H
