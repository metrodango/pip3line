/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "urlencodewidget.h"
#include "ui_urlencodewidget.h"

UrlEncodeWidget::UrlEncodeWidget(UrlEncode *ntransform, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::UrlEncodeWidget();
    if (ui == NULL) {
        qFatal("Cannot allocate memory for Ui::UrlEncodeWidget X{");
    }
    transform = ntransform;
    ui->setupUi(this);
    ui->includeLineEdit->setText(QString(transform->getInclude()));
    ui->percentLineEdit->setText(QString(transform->getPercentSign()));
    ui->excludeLineEdit->setText(QString(transform->getExclude()));

    connect(ui->includeLineEdit,SIGNAL(textChanged(QString)),this,SLOT(onIncludeChange(QString)));
    connect(ui->excludeLineEdit,SIGNAL(textChanged(QString)),this,SLOT(onExcludeChange(QString)));
    connect(ui->includeAllPushButton,SIGNAL(clicked()),this,SLOT(onIncludeAll()));
    connect(ui->percentLineEdit,SIGNAL(textChanged(QString)), this, SLOT(onPercentChange(QString)));
}

UrlEncodeWidget::~UrlEncodeWidget()
{
    delete ui;
}

void UrlEncodeWidget::onIncludeAll()
{
    ui->includeLineEdit->setText(UrlEncode::TEXTCHAR);
}

void UrlEncodeWidget::onExcludeChange(QString vals)
{
    transform->setExclude(vals.toUtf8());
}

void UrlEncodeWidget::onIncludeChange(QString vals)
{
    transform->setInclude(vals.toUtf8());
}

void UrlEncodeWidget::onPercentChange(QString character)
{
    QByteArray temp = character.toUtf8();

    if (temp.isEmpty())
        return;

    transform->setPercentSign(temp.at(0));

}
