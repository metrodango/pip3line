/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef MESSAGEDIALOG_H
#define MESSAGEDIALOG_H

#include "appdialog.h"
#include <QColor>
#include <QMutex>

namespace Ui {
class MessageDialog;
}

class MessageDialog : public AppDialog
{
    Q_OBJECT
    
    public:
        explicit MessageDialog(GuiHelper * guiHelper, QWidget *parent = nullptr);
        ~MessageDialog();
        long errors();
        long warnings();
        bool hasMessages();
        void setJustShowMessages(bool flag);

    public slots:
        void logWarning(const QString message, const QString source = QString());
        void logError(const QString message, const QString source = QString());
        void clearMessages();

    private:
        Q_DISABLE_COPY(MessageDialog)
        void addMessage(const QString &message, QColor color);
        QMutex lock;
        Ui::MessageDialog *ui;
        long warningCount;
        long errorCount;
};

#endif // MESSAGEDIALOG_H
