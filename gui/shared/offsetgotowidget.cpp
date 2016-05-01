/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "offsetgotowidget.h"
#include <QDebug>
#include <QApplication>
#include "../guihelper.h"
#include "guiconst.h"
using namespace GuiConst;

const QRegExp OffsetGotoWidget::offsetRegexp = QRegExp("^([=+-])?(0x|[oxn])?([0-9a-fA-F]{0,20})$");
const QString OffsetGotoWidget::TOOLTIP_TEXT = QObject::tr("<html><head/><body><p>First character defines relativity:<br/>"
                                                                  "'=' =&gt; absolute offset (default)<br/>"
                                                                  "+ =&gt; positive relative offset<br/>"
                                                                  "- =&gt; negative relative offset</p>"
                                                                  "<p>Second character defines the format:<br/>"
                                                                  "x/0x = hexadecimal number<br/>"
                                                                  "o = octal number<br/>"
                                                                  "n = decimal number<br/>"
                                                                  "Default is configured from the  global settings</p>"
                                                                  "<p>The range validity is between 0 and unsigned INT64_MAX</p>"
                                                                  "<p>Pressing enter/return will go to the offset<br/>"
                                                                  "shift+enter will also select everything since the last position</p></body></html>"
                                                                  );
const QString OffsetGotoWidget::PLACEHOLDER_TEXT = QObject::tr("Offset (ctrl+g)");

OffsetValidator::OffsetValidator(QObject *parent) :
    QValidator(parent)
{
}

QValidator::State OffsetValidator::validate(QString &input, int &) const
{
    if (input.isEmpty())
        return QValidator::Intermediate;

    QRegExp offsetRegExp = OffsetGotoWidget::offsetRegexp;
    if (offsetRegExp.indexIn(input) != -1) {
        if (!offsetRegExp.cap(3).isEmpty()) {
            bool ok = false;
            if (offsetRegExp.cap(2) == "o") {
                offsetRegExp.cap(3).toULongLong(&ok, 8);
            } else if (offsetRegExp.cap(2) == "n") {
                offsetRegExp.cap(3).toULongLong(&ok, 10);
            } else {
                offsetRegExp.cap(3).toULongLong(&ok, 16);
            }

            if (ok) {
                return QValidator::Acceptable;
            } else {
                return QValidator::Invalid;
            }

        }
        return QValidator::Intermediate;
    }
    return QValidator::Invalid;
}

OffsetGotoWidget::OffsetGotoWidget(GuiHelper *nguiHelper, QWidget *parent) :
    QLineEdit(parent)
{
    guiHelper = nguiHelper;
    OffsetValidator *validator = new(std::nothrow) OffsetValidator(this);
    if (validator == nullptr) {
        qFatal("Cannot allocate memory for OffsetValidator X{");
    }
    setValidator(validator);
  //  setMaximumWidth(100);
    setPlaceholderText(PLACEHOLDER_TEXT);
    setToolTip(TOOLTIP_TEXT);
}

void OffsetGotoWidget::onGoto(QString offsetString, bool select)
{
    setStyleSheet(qApp->styleSheet());
    if (offsetString.isEmpty()) {
        return;
    }

    setStyleSheet(qApp->styleSheet());

    if (offsetRegexp.indexIn(offsetString) != -1) {

        if (!offsetRegexp.cap(3).isEmpty()) {
            quint64 val = 0;

            bool ok = false;
            if (offsetRegexp.cap(2) == "o") {
                val = offsetRegexp.cap(3).toULongLong(&ok, 8);
            } else if (offsetRegexp.cap(2) == "n") {
                val = offsetRegexp.cap(3).toULongLong(&ok, 10);
            } else if (offsetRegexp.cap(2) == "x"){
                val = offsetRegexp.cap(3).toULongLong(&ok, 16);
            } else { // default from configuration
                val = offsetRegexp.cap(3).toULongLong(&ok, guiHelper->getDefaultOffsetBase());
            }

            if (ok) {
                bool neg = false;
                bool absolute = false;
                if (offsetRegexp.cap(1) == "-") {
                    neg = true;
                } else if (offsetRegexp.cap(1) != "+") { // not relative +
                    absolute = true;
                }

                emit gotoRequest(val,absolute, neg, select);
                return;
            } else {
                emit error("Invalid Offset value");
            }
        }
    } else {
        emit error("Invalid Offset specification");
    }

    setStyleSheet(GuiStyles::LineEditError);
}

void OffsetGotoWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return) {
        onGoto(text(),event->modifiers().testFlag(Qt::ShiftModifier));
        event->accept();
        return;
    }
    QLineEdit::keyPressEvent(event);
}

void OffsetGotoWidget::focusInEvent(QFocusEvent *event)
{
    if (event->gotFocus())
        selectAll();
}
