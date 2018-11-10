/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef STATEORCHESTRATOR_H
#define STATEORCHESTRATOR_H

#include <QObject>
#include <QStack>
#include <QTime>
#include <commonstrings.h>

class QThread;
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
        ~StateOrchestrator();
        QXmlStreamWriter *getWriter() const;
        bool initialize();
        bool isSaving();
        StateDialog * getStatusDialog(QWidget *parent = nullptr);
        StateStatusWidget *getStatusGui(QWidget *parent = nullptr);

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
        QThread *localthread;
        quint64 flags;
        QString actionName;
        QTime timer;
        QString fileName;
        QFile *file;
        QXmlStreamWriter * writer;
        QXmlStreamReader *reader;
};


#endif // STATEORCHESTRATOR_H
