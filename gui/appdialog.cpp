/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "appdialog.h"
#include "guihelper.h"
#include "loggerwidget.h"
#include "shared/guiconst.h"
#include <QDebug>
#include <QSettings>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QXmlStreamAttributes>
#include <QRect>
#include <QDesktopWidget>
#include <QApplication>

using namespace GuiConst;


AppStateObj::AppStateObj(AppDialog *diag) :
    dialog(diag)
{
    setName(dialog->metaObject()->className());
}

AppStateObj::~AppStateObj()
{

}

void AppStateObj::run()
{
    qDebug() << "Save/load" << name;
    if (flags & GuiConst::STATE_SAVE_REQUEST) {
        if (writer != nullptr)
            writer->writeStartElement(name);
        else {
            emit log(tr("Writer is null T_T"),this->metaObject()->className(), Pip3lineConst::LERROR);
            return;
        }
    } else if (!readNextStart()) // the check for reader == nullptr is already done in the function
        return;


    QByteArray sdata;
    if (flags & GuiConst::STATE_SAVE_REQUEST) {
        if (flags & GuiConst::STATE_LOADSAVE_DIALOG_POS) {
            writer->writeAttribute(GuiConst::STATE_WIDGET_GEOM, write(dialog->saveGeometry()));
            writer->writeAttribute(GuiConst::STATE_DIALOG_ISVISIBLE, write(dialog->isVisible()));
        }
    } else {
        if (flags & GuiConst::STATE_LOADSAVE_DIALOG_POS &&
                reader->name() == name) {
            QXmlStreamAttributes attrList = reader->attributes();
            sdata = readByteArray(attrList.value(GuiConst::STATE_WIDGET_GEOM));
            if (!sdata.isEmpty())
                dialog->restoreGeometry(sdata);

            if (attrList.hasAttribute(GuiConst::STATE_DIALOG_ISVISIBLE))
                dialog->setVisible(readBool(attrList.value(GuiConst::STATE_DIALOG_ISVISIBLE)));
        }
    }

    internalRun();

    genCloseElement();
}

void AppStateObj::internalRun()
{
//    if (reader != nullptr)
//        qDebug() << "AppStateObj:" << reader->name();
}


AppDialog::AppDialog(GuiHelper * nguiHelper, QWidget *parent) :
    QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint)
{
    guiHelper = nguiHelper;
    logger = guiHelper->getLogger();
    savedVisibleState = isVisible();
    attachedAction = nullptr;
    connect(guiHelper, &GuiHelper::appGoesIntoHidding, this, &AppDialog::onMainHiding);
    connect(guiHelper, &GuiHelper::appIsRising, this, &AppDialog::onMainRising);
    const QRect screen = QApplication::desktop()->screenGeometry();
    move(screen.center() - this->rect().center());
    //qDebug() << "created" << this;
}

AppDialog::~AppDialog()
{
    attachedAction = nullptr;
}

void AppDialog::attachAction(QAction *action)
{
    if (action->isCheckable())
        attachedAction = action;
    else
        logger->logError(tr("Action is not checkable, ignoring"),this->metaObject()->className());
}

BaseStateAbstract *AppDialog::getStateMngtObj()
{
    BaseStateAbstract *stateObj = new(std::nothrow) AppStateObj(this);
    if (stateObj == nullptr) {
        qFatal("Cannot allocate memory for AppStateObj X{");
    }

    return stateObj;
}

void AppDialog::onMainHiding()
{
    savedVisibleState = isVisible();
    if (savedVisibleState)
        hide();
}

void AppDialog::onMainRising()
{
    if (savedVisibleState) {
        show();
    }
}

void AppDialog::showEvent(QShowEvent *event)
{
    if (!savedPos.isNull())
        move(savedPos);

    if (attachedAction != nullptr)
        attachedAction->setChecked(true);
    QDialog::showEvent(event);
}

void AppDialog::hideEvent ( QHideEvent * event)
{
    savedPos = pos();
    if (attachedAction != nullptr)
        attachedAction->setChecked(false);
    QDialog::hideEvent(event);
}
