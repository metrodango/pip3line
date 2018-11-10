/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "stateorchestrator.h"
#include "basestateabstract.h"
#include <QTimer>
#include <QThread>
#include <QDebug>
#include <QSetIterator>
#include <QFile>
#include <QDir>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QTemporaryFile>
#include <QFileInfo>
#include "../shared/guiconst.h"
#include "statedialog.h"
#include "statestatuswidget.h"

StateOrchestrator::StateOrchestrator(QString fileName, quint64 flags) :
    QObject(nullptr),
    localthread(nullptr),
    flags(flags),
    fileName(fileName),
    file(nullptr),
    writer(nullptr),
    reader(nullptr)
{
    localthread = new(std::nothrow) QThread();
    if (localthread == nullptr) {
        qFatal("Cannot allocate memory for StateOrchestrator local thread X{");
    }
    moveToThread(localthread);
    localthread->start();

    if (isSaving()) {
        actionName = GuiConst::STATE_ACTION_SAVE_STR;
    } else {
        actionName = GuiConst::STATE_ACTION_RESTORE_STR;
    }
   // qDebug() << this << "created";
}

StateOrchestrator::~StateOrchestrator()
{
    stop();
    delete writer;
    writer = nullptr;
    delete reader;
    reader = nullptr;
    delete file;
    file = nullptr;
}

bool StateOrchestrator::start()
{
    timer.start();
    emit log(tr("%1 state: %2").arg(actionName).arg(fileName),metaObject()->className(), Pip3lineConst::PLSTATUS);
    execNext();

    return true;
}

void StateOrchestrator::stop()
{
    localthread->exit();
    if (!localthread->wait())
        qCritical() << tr("Could not stop the local thread for StateOrchestrator T_T");
    delete localthread;
    if (!executionStack.isEmpty()) {
         qCritical() << tr("Execution stack is not empty T_T");
         while (!executionStack.isEmpty()) {
             delete executionStack.pop();
         }
    }
}


void StateOrchestrator::addState(BaseStateAbstract *stateobj)
{
   // qDebug() << "Adding state" << stateobj;
    stateobj->setReader(reader);
    stateobj->setWriter(writer);
    stateobj->setFlags(flags);
    connect(stateobj, &BaseStateAbstract::finished, this, &StateOrchestrator::execNext,Qt::QueuedConnection);
    connect(stateobj, &BaseStateAbstract::addNewState, this, &StateOrchestrator::addState, Qt::QueuedConnection);
    connect(stateobj, &BaseStateAbstract::log, this, &StateOrchestrator::log);
    executionStack.push(stateobj);
}


void StateOrchestrator::execNext()
{
    if (!executionStack.isEmpty()) {
        BaseStateAbstract *stateobj = executionStack.pop();
        emit statusupdate();
        // emit log(tr("%1 %2").arg(actionName).arg(stateobj->getName()),metaObject()->className() , Pip3lineConst::LSTATUS);
        QTimer::singleShot(50, stateobj, SLOT(start()));
    } else {
        emit log(tr("%1 state performed in %2ms").arg(actionName).arg(timer.restart()),metaObject()->className() , Pip3lineConst::PLSTATUS);
        if (writer != nullptr) {
            writer->writeEndElement();
            writer->writeEndDocument();
        }
        onFinished();
    }
}

void StateOrchestrator::onFinished()
{
    if (isSaving()) {
        // committing any data left;
        file->flush();
        if (file->size() > 0) {
            if (QFile::exists(fileName)) { // removing destination file if already exist
                QFile::remove(fileName);
            }

            if (!file->copy(fileName)) { // copying the temp file into the actual file
                emit log(tr("Error while copying the temp file to the final state file: %1").arg(file->errorString()), metaObject()->className(), Pip3lineConst::LERROR);
            }
        }
    }
    emit finished();
}

QXmlStreamWriter *StateOrchestrator::getWriter() const
{
    return writer;
}

bool StateOrchestrator::initialize()
{
    bool ret = false;

    if (isSaving()) {

        QFileInfo fi(fileName);
        QTemporaryFile *tfile = new(std::nothrow) QTemporaryFile(fi.absolutePath().append(QDir::separator()));
        file = tfile;

        if (tfile == nullptr) {
            qFatal("Cannot allocate memory for QTemporaryFile X{");
        }

        ret = tfile->open();
        if (!ret) {
            emit log(tr("Cannot open temporary state file for saving: %1").arg(tfile->errorString()),metaObject()->className(), Pip3lineConst::LERROR);
            return false;
        }
        writer = new(std::nothrow) QXmlStreamWriter(tfile);

        if (writer == nullptr) {
            qFatal("Cannot allocate memory for QXmlStreamWriter X{");
        }

        writer->writeStartDocument();
        writer->writeStartElement(GuiConst::STATE_PIP3LINE_DOC);

    } else {
        file = new(std::nothrow) QFile(fileName); // file is automatically deleted when the orchestrator is deleted
        if (file == nullptr) {
            qFatal("Cannot allocate memory for QFile X{");
        }

        ret = file->open(QIODevice::ReadOnly);
        if (!ret) {
            emit log(tr("Cannot open state file for loading: %1").arg(file->errorString()),metaObject()->className(), Pip3lineConst::LERROR);
            return false;
        }
        reader = new(std::nothrow) QXmlStreamReader(file);

        if (reader == nullptr) {
            qFatal("Cannot allocate memory for QXmlStreamReader X{");
        }

        if (!reader->readNextStartElement()) {
            emit log(tr("XML document seems empty"),metaObject()->className(), Pip3lineConst::LERROR);
            return false;
        } else if (reader->name() != GuiConst::STATE_PIP3LINE_DOC) {
            emit log(tr("This is not a Pip3line state document (%1)").arg(reader->name().toString()),metaObject()->className(), Pip3lineConst::LERROR);
            return false;
        }
    }

    return true;
}

bool StateOrchestrator::isSaving()
{
    return (flags & GuiConst::STATE_SAVE_REQUEST);
}

StateDialog *StateOrchestrator::getStatusDialog(QWidget *parent)
{
    StateDialog * stateDialog = new(std::nothrow) StateDialog(parent);
    if (stateDialog == nullptr) {
        qFatal("Cannot allocate memory for StateDialog X{");
    }
    stateDialog->setModal(true);

    connect(this, &StateOrchestrator::log,stateDialog, &StateDialog::log);
    connect(this, &StateOrchestrator::statusupdate, stateDialog, &StateDialog::update);

    return stateDialog;
}

StateStatusWidget *StateOrchestrator::getStatusGui(QWidget * parent)
{
    StateStatusWidget *w = new(std::nothrow) StateStatusWidget(parent);
    if (w == nullptr) {
        qFatal("Cannot allocate memory for StateStatusWidget X{");
    }
    w->setMessage(actionName);
    connect(this, &StateOrchestrator::log, w,&StateStatusWidget::log);

    connect(this, &StateOrchestrator::statusupdate, w, &StateStatusWidget::update);

    return w;
}


