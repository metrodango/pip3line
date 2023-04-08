/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#include "xmlquerywidget.h"
#include "ui_xmlquerywidget.h"

XmlQueryWidget::XmlQueryWidget(XmlQuery *ntransform, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::XmlQueryWidget();
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::XmlQueryWidget X{");
    }
    transform = ntransform;
    ui->setupUi(this);
    ui->queryPlainTextEdit->appendPlainText(transform->getQueryString());
    connect(ui->submitPushButton, &QPushButton::clicked, this, &XmlQueryWidget::onQuerySubmit);
}

XmlQueryWidget::~XmlQueryWidget()
{
    delete ui;
}

void XmlQueryWidget::onQuerySubmit()
{
    if (!ui->queryPlainTextEdit->toPlainText().isEmpty()) {
        transform->setQueryString(ui->queryPlainTextEdit->toPlainText());
    }
}
