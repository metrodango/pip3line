/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#ifndef APPDIALOG_H
#define APPDIALOG_H

#include <QDialog>
#include <QPoint>
#include <QSettings>
#include "shared/guiconst.h"
#include "state/basestateabstract.h"

class GuiHelper;
class LoggerWidget;
class QAction;

class AppDialog : public QDialog
{
        Q_OBJECT
    public:
        explicit AppDialog(GuiHelper * guiHelper, QWidget *parent = nullptr);
        virtual ~AppDialog();
        void attachAction(QAction * action);
        virtual BaseStateAbstract *getStateMngtObj();
    signals:
        void hiding();
    protected slots:
        void onMainHiding();
        void onMainRising();
    protected:
        void showEvent(QShowEvent * event);
        GuiHelper * guiHelper;
        bool savedVisibleState;
        QPoint savedPos;
        LoggerWidget *logger;
        void hideEvent(QHideEvent *event);
        QAction * attachedAction;
    private:
        Q_DISABLE_COPY(AppDialog)
};

class AppStateObj : public BaseStateAbstract
{
        Q_OBJECT
    public:
        explicit AppStateObj(AppDialog *diag);
        virtual ~AppStateObj();
        virtual void run();
    protected:
        virtual void internalRun();
        AppDialog *dialog;
    private:
        Q_DISABLE_COPY(AppStateObj)
};

#endif // APPDIALOG_H
