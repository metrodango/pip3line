/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#include "masterthread.h"
#include <QFile>
#include "../tools/textprocessor.h"
#include "../tools/binaryprocessor.h"
#include <QtCore/QCoreApplication>

MasterThread::MasterThread(bool nbinaryInput, bool hideErrorsFlag, bool verboseFlag, QObject *parent) :
    QThread(parent)
{
    verbose = verboseFlag;
    hideErrors = hideErrorsFlag;
    binaryInput = nbinaryInput;
    processor = nullptr;
    singleWay = TransformAbstract::INBOUND;
    errlog = new(std::nothrow) QTextStream(stderr);
    if (errlog == nullptr) {
        qFatal("Cannot allocate memory for errlog X{");
    }
    messlog = new(std::nothrow) QTextStream(stdout);
    if (messlog == nullptr) {
        qFatal("Cannot allocate memory for messlog X{");
    }

    connect(&transformFactory, &TransformMgmt::error, this, &MasterThread::logError);
    connect(&transformFactory, &TransformMgmt::status, this, &MasterThread::logStatus);
    transformFactory.initialize(QCoreApplication::applicationDirPath());
}

MasterThread::~MasterThread()
{
    delete processor;
    delete errlog;
    delete messlog;

}

void MasterThread::run()
{
    QFile fin;
    QFile fout;

    fin.open(stdin, QFile::ReadOnly | QIODevice::Unbuffered);
    fout.open(stdout,QFile::WriteOnly | QIODevice::Unbuffered);

    if (binaryInput) {
        processor = new(std::nothrow) BinaryProcessor(&transformFactory);
        if (processor == nullptr) {
            qFatal("Cannot allocate memory for BinaryProcessor X{");
        }

    } else {
        processor = new(std::nothrow) TextProcessor(&transformFactory);
        if (processor == nullptr) {
            qFatal("Cannot allocate memory for TextProcessor X{");
        }
    }
    processor->setInput(&fin);
    processor->setOutput(&fout);
    if (!hideErrors)
        connect(processor, &Processor::error, this, &MasterThread::logError);

    if (verbose)
        connect(processor, &Processor::status, this, &MasterThread::logStatus);

    if (!transformName.isEmpty() && processor->configureFromName(transformName, singleWay)) {
        processor->start();
        processor->wait();
    } else if (!confFileName.isEmpty() && processor->configureFromFile(confFileName)) {
        processor->start();
        processor->wait();
    }

    delete processor;
    processor = nullptr;

    QCoreApplication::exit(0);
}

void MasterThread::setConfigurationFile(const QString &fileName, bool inbound)
{
    confFileName = fileName;
    if (inbound) {
        singleWay = TransformAbstract::INBOUND;
    } else {
        singleWay = TransformAbstract::OUTBOUND;
    }
}

void MasterThread::setTransformName(const QString &ntransformName, bool inbound)
{
    transformName = ntransformName;
    if (inbound) {
        singleWay = TransformAbstract::INBOUND;
    } else {
        singleWay = TransformAbstract::OUTBOUND;
    }
}

void MasterThread::logError(const QString mess, const QString &source)
{
    if (!hideErrors) {
        if (!source.isEmpty()) {
            *errlog << source << ": ";
        }
        *errlog << mess << QTENDL;
    }
}

void MasterThread::logStatus(const QString mess, const QString &source)
{
    if (verbose) {
        if (!source.isEmpty()) {
            *messlog << source << ": ";
        }
        *messlog << mess << QTENDL;
    }
}
