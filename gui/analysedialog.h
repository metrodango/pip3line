/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef ANALYSE_H
#define ANALYSE_H

#include "appdialog.h"
#include <QMutex>
#include <QMenu>

namespace Ui {
class AnalyseDialog;
}

class AnalyseDialog : public AppDialog
{
        Q_OBJECT
        
    public:
        explicit AnalyseDialog(GuiHelper * guiHelper, QWidget *parent = nullptr);
        ~AnalyseDialog();
        BaseStateAbstract *getStateMngtObj();
        QString getInputValue();
        QString getOutputValue();
        void setInputValue(const QString & val);
        void setOutputValue(const QString & val);
    signals:
        void results(QString);
private slots:
        void gatheringResults(QString);
        void cleaning();
        void onInputRightClick(QPoint pos);
        void on_actionImportFile_triggered();
        void onCharsetAnalysis();
        void charSetAnalysis();
        void onCharFrequencyAnalysis();
        void charFrequencyAnalysis();
        void onHashGuess();
        void hashGuess();

    private:
        Q_DISABLE_COPY(AnalyseDialog)
        QString potentialHashes(int length);
        QList<QByteArray> getInput();
        Ui::AnalyseDialog *ui;
        QMutex lock;
        QMenu * plainTextContextMenu;
};

class AnalyseDialogStateObj : public AppStateObj
{
        Q_OBJECT
    public:
        explicit AnalyseDialogStateObj(AnalyseDialog *diag);
        ~AnalyseDialogStateObj();
    private:
        Q_DISABLE_COPY(AnalyseDialogStateObj)
        void internalRun();
};

#endif // ANALYSE_H
