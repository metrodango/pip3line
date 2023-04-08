/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#ifndef OFFSETGOTOWIDGET_H
#define OFFSETGOTOWIDGET_H

#include <QLineEdit>
#include <QKeyEvent>
#include <QRegExp>
#include <QValidator>

class GuiHelper;

class OffsetValidator : public QValidator
{
        Q_OBJECT
    public:
        explicit OffsetValidator(QObject *parent = nullptr);
        QValidator::State validate(QString & input, int & pos) const;

};

class OffsetGotoWidget : public QLineEdit
{
        Q_OBJECT
    public:
        static const QRegExp offsetRegexp;
        explicit OffsetGotoWidget(GuiHelper *guiHelper,QWidget *parent = nullptr);
    signals:
        void gotoRequest(quint64 offset, bool absolute, bool sign, bool select);
        void error(QString mess);
    private slots:
        void onGoto(QString offsetString, bool select);
    private:
        Q_DISABLE_COPY(OffsetGotoWidget)
        static const QString TOOLTIP_TEXT;
        static const QString PLACEHOLDER_TEXT;
        void keyPressEvent(QKeyEvent * event);
        void focusInEvent(QFocusEvent * event);
        GuiHelper *guiHelper;
        
};

#endif // OFFSETGOTOWIDGET_H
