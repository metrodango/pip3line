/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#include "messagepanelwidget.h"
#include "ui_messagepanelwidget.h"
#include <QCursor>

MessagePanelWidget::MessagePanelWidget(QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::MessagePanelWidget();
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::MessagePanelWidget X{");
    }
    ui->setupUi(this);
    this->setVisible(false);
    connect(ui->pushButton, &QPushButton::clicked, this, &MessagePanelWidget::closeWidget);
}

MessagePanelWidget::~MessagePanelWidget()
{
    delete ui;
}

QString MessagePanelWidget::toHTML()
{
    if (ui->textEdit->toPlainText().size() > 0)
        return ui->textEdit->toHtml();
    else
        return QString();
}

void MessagePanelWidget::setHTML(QString html)
{
    ui->textEdit->clear();
    if (!html.isEmpty())
        ui->textEdit->setHtml(html);
}

void MessagePanelWidget::logWarning(const QString message, const QString )
{
    addMessage(message,Qt::blue);
}

void MessagePanelWidget::logError(const QString message, const QString )
{
    addMessage(message,Qt::red);
}

void MessagePanelWidget::logStatus(const QString message, const QString )
{
    addMessage(message,Qt::black);
}

void MessagePanelWidget::log(QString mess, QString , Pip3lineConst::LOGLEVEL level)
{
    QColor color = Qt::black;
    if (level == Pip3lineConst::LERROR)
        color = Qt::red;
    else if (level == Pip3lineConst::LWARNING)
        color = Qt::blue;

    addMessage(mess,color);
}

void MessagePanelWidget::closeWidget()
{
    ui->textEdit->clear();
    this->setVisible(false);
}

void MessagePanelWidget::clear()
{
    ui->textEdit->clear();
}

void MessagePanelWidget::addMessage(const QString &message, QColor color)
{
    ui->textEdit->setTextColor(color);
    ui->textEdit->append(message);
    this->setVisible(true);
}
