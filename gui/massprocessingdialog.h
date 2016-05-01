/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef MASSPROCESSINGDIALOG_H
#define MASSPROCESSINGDIALOG_H

#include "appdialog.h"

#include <QMutex>
#include <QTextEdit>
#include "messagedialog.h"
#include <transformchain.h>
#include "loggerwidget.h"
#include <QTimer>
#include <QSettings>

class TransformsGui;
class GuiHelper;
class Processor;
class TransformMgmt;
class ServerAbstract;
class QIODevice;
class LoggerWidget;

namespace Ui {
class MassProcessingDialog;
}

class MassProcessingDialog : public AppDialog
{
        Q_OBJECT
        
    public:
        explicit MassProcessingDialog(GuiHelper * helper,TransformsGui *tGui);
        ~MassProcessingDialog();
    public slots:
        void setTranformChain(const QString &chainConf);
    signals:
        void finished();
        void newServerTransformChain(QString chainConf);
    private slots:
        void selectInputFile();
        void selectOutputFile();
        void on_processingPushButton_clicked();
        void releasingThread();
        void cleaningMem();
        void stats();
        void onInputChanged(int index);
        void onOutputChanged(int index);
        void onSeparatorChanged(QString hexSep);
        void refreshTransformConf();
        void onUseSocketForOutput(bool checked);
        void clearOnScreenOutput();
        void restartCurrentServer();
        void stopCurrentServer();
        void refreshIPs();
        void onKeepSynchronize(bool checked);
        void onLogsEnabled(bool checked);

    private:
        Q_DISABLE_COPY(MassProcessingDialog)
        enum ReadingTypes {LINES = 0, BLOCK};
        void deleteCurrentServer();
        QIODevice * getInput();
        QIODevice * getOutput();

        Ui::MassProcessingDialog *ui;
        QString transformConf;
        Processor * processor;
        MessageDialog errorDialog;
        QMutex threadMutex;
        QIODevice *currentInput;
        QIODevice *currentOutput;
        TransformMgmt * transformFactory;
        QByteArray tempManualInput;
        TransformsGui * tGui;
        QTimer statTimer;
        ServerAbstract *server;

};

#endif // MASSPROCESSINGDIALOG_H
