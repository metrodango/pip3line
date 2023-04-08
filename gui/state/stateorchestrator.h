/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#ifndef STATEORCHESTRATOR_H
#define STATEORCHESTRATOR_H

#include <QElapsedTimer>
#include <QObject>
#include <QStack>
#include <QThread>
#include <QTime>
#include <commonstrings.h>

class BaseStateAbstract;
class QFile;
class QXmlStreamWriter;
class QXmlStreamReader;
class StateDialog;
class QWidget;
class StateStatusWidget;

class StateOrchestrator : public QObject
{
        Q_OBJECT
    public:
        explicit StateOrchestrator(QString fileName, quint64 flags);
        explicit StateOrchestrator(QByteArray config, quint64 flags);
        ~StateOrchestrator();
        QXmlStreamWriter *getWriter() const;
        bool initialize();
        bool isSaving();
        StateDialog * getStatusDialog(QWidget *parent = nullptr);
        StateStatusWidget *getStatusGui(QWidget *parent = nullptr);
        QByteArray getConfigData() const;

    public slots:
        bool start();
        void stop();
        void addState(BaseStateAbstract * stateobj);
        void execNext();
    signals:
        void log(QString message, QString source, Pip3lineConst::LOGLEVEL level);
        void finished();
        void statusupdate();
    private:
        Q_DISABLE_COPY(StateOrchestrator)
        void onFinished();
        QStack<BaseStateAbstract *> executionStack;
        QThread localthread;
        quint64 flags;
        QString actionName;
        QElapsedTimer timer;
        QString fileName;
        QFile *file;
        QXmlStreamWriter * writer;
        QXmlStreamReader *reader;
        QByteArray configData;
};


#endif // STATEORCHESTRATOR_H
