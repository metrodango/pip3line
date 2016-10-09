/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "threadedprocessor.h"
#include <QHashIterator>
#include <QDebug>
#if QT_VERSION >= 0x050000
#include <QtConcurrent>
#endif
#include <QtConcurrentRun>

TransformRequest::TransformRequest(TransformAbstract *ntransform, const QByteArray &in, quintptr nptid, bool takeOwnerShip) :
    inputData(in),
    multiProcessing(false)
{
    transform = ntransform;
    connect(transform, SIGNAL(error(QString,QString)), this, SLOT(logError(QString,QString)),Qt::DirectConnection);
    connect(transform, SIGNAL(warning(QString,QString)), this, SLOT(logWarning(QString,QString)),Qt::DirectConnection);
    deleteObject = takeOwnerShip;
    if (ptid == 0)
        ptid = (quintptr) this;
    else
        ptid = nptid;
}

TransformRequest::TransformRequest(TransformAbstract *ntransform, const QList<QByteArray> &ins, quintptr nptid, bool takeOwnerShip) :
    inputDatas(ins),
    multiProcessing(true)
{
    transform = ntransform;
    connect(transform, SIGNAL(error(QString,QString)), this, SLOT(logError(QString,QString)),Qt::DirectConnection);
    connect(transform, SIGNAL(warning(QString,QString)), this, SLOT(logWarning(QString,QString)),Qt::DirectConnection);
    deleteObject = takeOwnerShip;
    if (ptid == 0)
        ptid = (quintptr) this;
    else
        ptid = nptid;

    //qDebug() << "Request" << this << "created";

}

TransformRequest::~TransformRequest()
{
    if (deleteObject)
        delete transform;
    transform = nullptr;

    //qDebug() << this << "deleted";
}

void TransformRequest::runRequest()
{
    if (transform != nullptr) {
        if (multiProcessing) {
            for (int i = 0 ; i < inputDatas.size(); i++) {
                outputData.clear();
                transform->transform(inputDatas.at(i),outputData);
                outputDatas.append(outputData);
            }
        } else {
            transform->transform(inputData,outputData);
        }
    } else {
        qWarning() << "[TransformRequest] transform is null, ignoring";
    }

 //   qDebug() << "Processor fisnished: " << this <<  " exec time: " << timer.elapsed() << "ms";

    if (multiProcessing) {
        emit finishedProcessing(outputDatas, messagesList);
    } else {
        emit finishedProcessing(outputData, messagesList);
    }
    emit endOfProcess();
}

quintptr TransformRequest::getptid() const
{
    return ptid;
}

void TransformRequest::logError(QString message, QString source)
{
    logMessage(message, source, LERROR);
}

void TransformRequest::logWarning(QString message, QString source)
{
    logMessage(message, source, LWARNING);
}

void TransformRequest::logStatus(QString message, QString source)
{
    logMessage(message, source, LSTATUS);
}


void TransformRequest::logMessage(QString message, QString source, LOGLEVEL level)
{
    Message m;
    m.level = level;
    m.message = message;
    m.source = source;
    messagesList.append(m);
}

ThreadedProcessor::ThreadedProcessor(QObject *parent) :
    QObject(parent)
{
    moveToThread(&workerThread);
    workerThread.start();
}

ThreadedProcessor::~ThreadedProcessor()
{
    workerThread.quit();
    workerThread.wait();
    if (!currentRunning.isEmpty()) {
        QHashIterator<TransformRequest *, QFuture<void> > i(currentRunning);
        while (i.hasNext()) {
            i.next();
            TransformRequest * request = i.key(); // there might be a race ccondition here, not sure
            QFuture<void> fut = i.value();
            request->blockSignals(true);

            fut.waitForFinished();

            delete request;
        }
        currentRunning.clear();
    }
}

void ThreadedProcessor::processRequest(TransformRequest *request)
{

    quintptr source = request->getptid();
    if (isSourceRunning(source)) {
        if (waitingRequests.contains(source)) { // if the source has send any unexecuted request, just erase it
            qDebug() << "One request is already waiting for execution, replacing it";
            delete waitingRequests.take(request->getptid());
        }
        waitingRequests.insert(source,request);
    } else {
        startRequest(request);
    }
}

void ThreadedProcessor::onRequestFinished()
{
    TransformRequest * tr = static_cast<TransformRequest *>(sender());
    if (tr != nullptr) {
        quintptr source = tr->getptid();
        if (currentRunning.contains(tr)) {
            QFuture<void> fut = currentRunning.take(tr);
            if (!fut.isFinished()) {
                qDebug("[ThreadedProcessor::onRequestFinished] something is fishy, the request thread has not terminated yet T_T");
                fut.waitForFinished();
            }
        } else {
            qWarning("[ThreadedProcessor::onRequestFinished] value not found when removing child T_T");
        }
        if (waitingRequests.contains(source)) {
            startRequest(waitingRequests.take(source));
        }
        tr->deleteLater();
    } else {
        qWarning("[ThreadedProcessor::onRequestFinished] null sender request T_T");
    }
}

void ThreadedProcessor::startRequest(TransformRequest *request)
{
    connect(request, SIGNAL(endOfProcess()), this, SLOT(onRequestFinished()),Qt::QueuedConnection);
    QFuture<void> future = QtConcurrent::run(request, &TransformRequest::runRequest);
    currentRunning.insert(request, future);
}

bool ThreadedProcessor::isSourceRunning(quintptr source)
{
    QHashIterator<TransformRequest *, QFuture<void> > i(currentRunning);
    while (i.hasNext()) {
        i.next();
        if (i.key()->getptid() == source)
            return true;
    }
    return false;
}
