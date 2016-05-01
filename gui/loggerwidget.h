/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef LOGGERWIDGET_H
#define LOGGERWIDGET_H

#include <QWidget>
#include <QMutex>
#include <QTextStream>
#include <QShowEvent>
#include <commonstrings.h>

namespace Ui {
class LoggerWidget;
}

class LoggerWidget : public QWidget
{
        Q_OBJECT
        
    public:
        explicit LoggerWidget(QWidget *parent = 0);
        ~LoggerWidget();
        void showEvent ( QShowEvent * event );
        bool hasUncheckedError() const;
    signals:
        void errorRaised();
        void cleanStatus();
    public slots:
        void logError(const QString &message, const QString &source = QString());
        void logWarning(const QString &message,const QString &source = QString());
        void logStatus(const QString &message,const QString &source = QString());
        void logMessage(const QString &message,const QString &source = QString(), Pip3lineConst::LOGLEVEL level = Pip3lineConst::LSTATUS);
    private:
        Q_DISABLE_COPY(LoggerWidget)
        void addMessage(const QString &message, const QString &source, Pip3lineConst::LOGLEVEL level);
        Ui::LoggerWidget *ui;
        QMutex lock;
        bool uncheckedError;
};

#endif // LOGGERWIDGET_H
