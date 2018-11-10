/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "processor.h"
#include "QMutexLocker"
#include <QXmlStreamReader>
#include <transformmgmt.h>
#include <QIODevice>
#include <QMutex>
#include <QDebug>

const char Processor::DEFAULT_SEPARATOR = '\n';

Processor::Processor(TransformMgmt * tFactory, QObject *parent) : QThread(parent)
{
    transformFactory = tFactory;
    outputLock = nullptr;
    out = nullptr;
    in = nullptr;
    separator = DEFAULT_SEPARATOR;
    encode = false;
    decode = false;
}

Processor::~Processor()
{
    qDebug() << "Destroying " << this;
    clearChain();
}


bool Processor::configureFromFile(const QString &fileName)
{
    return setTransformsChain(transformFactory->loadConfFromFile(fileName));
}

bool Processor::configureFromName(const QString &name, TransformAbstract::Way way)
{
    TransformChain chain;
    TransformAbstract * t = transformFactory->getTransform(name);
    if (t != nullptr) {
        t->setWay(way);
        chain.append(t);
    }
    else
        return false;
    return setTransformsChain(chain);
}

bool Processor::setTransformsChain(TransformChain ntlist)
{

    QMutexLocker lock(&tranformsLock);
    clearChain();
    tlist = ntlist;
    if (tlist.size() == 0) {
        emit error(tr("No Transformation loaded"), "Processor");
        return false;
    } else {
        for (int i = 0; i < tlist.size(); i++) {
            //qDebug() << "Tranformation loaded" << tlist.at(i)->name();
            connect(tlist.at(i), &TransformAbstract::error, this, &Processor::logError);
            connect(tlist.at(i), &TransformAbstract::warning, this, &Processor::logError);
        }
        emit status(tr("%1 transformations loaded").arg(tlist.size()), "Processor");
    }
    return true;
}

bool Processor::setTransformsChain(const QString &xmlConf)
{
    QXmlStreamReader streamIn(xmlConf);
    return setTransformsChain(transformFactory->loadConfFromXML(&streamIn));
}

void Processor::setOutputMutex(QMutex *mutex)
{
    outputLock = mutex;
}

void Processor::setOutput(QIODevice *nout)
{
    out = nout;
}

void Processor::setInput(QIODevice *nin)
{
    in = nin;
}

void Processor::clearOutputMutex()
{
    outputLock = nullptr;
}

void Processor::setSeparator(char c)
{
    separator = c;
}

ProcessingStats Processor::getStats()
{
    QMutexLocker lock(&statsLock);
    return stats;
}

void Processor::setEncoding(bool flag)
{
    encode = flag;
}

void Processor::setDecoding(bool flag)
{
    decode = flag;
}

void Processor::logError(const QString mess, const QString id)
{
    statsLock.lock();
    stats.incrErrorsCount();
    statsLock.unlock();
    emit error(mess, id);
}

void Processor::logStatus(const QString mess, const QString id)
{
    statsLock.lock();
    stats.incrStatusCount();
    statsLock.unlock();
    emit status(mess, id);
}

void Processor::writeBlock(const QByteArray &data)
{

    statsLock.lock();
    stats.incrInBlocks();
    statsLock.unlock();
    QByteArray temp = data;
    QByteArray temp2;
    QByteArray * outputval = nullptr;
    int i = 0;

    if (decode)
        temp = QByteArray::fromBase64(temp);

    for (i = 0; i < tlist.size(); i++) {
        if (i % 2 == 0)
            tlist.at(i)->transform(temp,temp2);
        else
            tlist.at(i)->transform(temp2,temp);
    }
    if (i % 2 == 0)
        outputval = &temp;
    else
        outputval = &temp2;

    if (!outputval->isEmpty()) {
        if (encode)
            *outputval = outputval->toBase64();
        outputval->append(separator);
        if (outputLock != nullptr)
            outputLock->lock();

        while (outputval->size() > 0) {
            qint64 whatHasBeenDone = out->write(*outputval);
            if (whatHasBeenDone < 0) {
                logError(out->errorString(), this->metaObject()->className());
                break;
            }

            if (whatHasBeenDone < outputval->size()) {
                *outputval = outputval->remove(0,static_cast<int>(whatHasBeenDone));
            } else {
                qCritical() << tr("[Processor::writeBlock] bytes written ");
                outputval->clear();
            }


        }

        if (outputLock != nullptr)
            outputLock->unlock();
        statsLock.lock();
        stats.incrOutBlocks();
        statsLock.unlock();
    }
}

void Processor::clearChain()
{
    while (!tlist.isEmpty())
        delete tlist.takeLast();
}
