/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#ifndef MESSAGEPANELWIDGET_H
#define MESSAGEPANELWIDGET_H

#include <QWidget>
#include <commonstrings.h>

namespace Ui {
class MessagePanelWidget;
}

class MessagePanelWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit MessagePanelWidget(QWidget *parent = nullptr);
        ~MessagePanelWidget();
        QString toHTML();
        void setHTML(QString html);
    public slots:
        void logWarning(const QString message, const QString source = QString());
        void logError(const QString message, const QString source = QString());
        void logStatus(const QString message, const QString source = QString());
        void log(QString mess,QString source,Pip3lineConst::LOGLEVEL level);
        void addMessage(const QString &message, QColor color);
        void closeWidget();
        void clear();
    private:
        Q_DISABLE_COPY(MessagePanelWidget)
        Ui::MessagePanelWidget *ui;
};

#endif // MESSAGEPANELWIDGET_H
