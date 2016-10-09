/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "loggerwidget.h"
#include "ui_loggerwidget.h"
#include <QMutexLocker>
#include <QTime>
#include <QDebug>
#include <QApplication>

using namespace Pip3lineConst;

LoggerWidget::LoggerWidget(QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::LoggerWidget();
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::LoggerWidget X{");
    }
    ui->setupUi(this);

    uncheckedError = false;
}

LoggerWidget::~LoggerWidget()
{
    delete ui;
}

void LoggerWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    uncheckedError = false;
    emit cleanStatus();
}

bool LoggerWidget::hasUncheckedError() const
{
    return uncheckedError;
}

void LoggerWidget::logError(const QString &message, const QString &source)
{
    addMessage(message,source,LERROR);
}

void LoggerWidget::logWarning(const QString &message, const QString &source)
{
    addMessage(message,source,LWARNING);
}

void LoggerWidget::logStatus(const QString &message, const QString &source)
{
    addMessage(message,source,LSTATUS);
}

void LoggerWidget::logMessage(const QString &message, const QString &source, Pip3lineConst::LOGLEVEL level)
{
    addMessage(message,source,level);
}


void LoggerWidget::addMessage(const QString &message, const QString &source, LOGLEVEL level)
{
    QMutexLocker locking(&lock);
    QString fmess;
    QColor color;

    if (!source.isEmpty())
        fmess.append(source).append(": ").append(message);
    else
        fmess.append(message);

    switch (level) {
        case LERROR:
            color = Qt::red;
            qCritical() << fmess;
            if (!(uncheckedError && isVisible())) {
                uncheckedError = true;
                emit errorRaised();
            }
            break;
        case LWARNING:
            color = Qt::blue;
            qWarning() << fmess;
            break;
        default:
            color = QApplication::palette().windowText().color();
#if QT_VERSION >= 0x050500
            qInfo() << fmess;
#else
            qWarning() << fmess;
#endif

            break;

    }

    ui->messagesTextEdit->setTextColor(color);
    ui->messagesTextEdit->append(fmess.prepend(QString("[%1]").arg(QTime::currentTime().toString())));
}
