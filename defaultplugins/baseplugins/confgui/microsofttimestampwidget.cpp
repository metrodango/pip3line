/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "microsofttimestampwidget.h"
#include "ui_microsofttimestampwidget.h"

MicrosoftTimestampWidget::MicrosoftTimestampWidget(MicrosoftTimestamp *ntransform, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::MicrosoftTimestampWidget();
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::MicrosoftTimestampWidget X{");
    }
    transform = ntransform;
    ui->setupUi(this);

    if (transform->getTZ() == MicrosoftTimestamp::TZ_UTC) {
        ui->utcRadioButton->setChecked(true);
    } else {
        ui->localTimeRadioButton->setChecked(true);
    }

    ui->formatLineEdit->setText(transform->getDateFormat());
    connect(ui->formatLineEdit, SIGNAL(textChanged(QString)), this, SLOT(formatChanged(QString)));
    connect(ui->nanoLineEdit, SIGNAL(textChanged(QString)), SLOT(onNSChanged(QString)));
    connect(ui->localTimeRadioButton, SIGNAL(toggled(bool)), SLOT(localTzToggled(bool)));
}

MicrosoftTimestampWidget::~MicrosoftTimestampWidget()
{
    delete ui;
}

void MicrosoftTimestampWidget::formatChanged(QString format)
{
    transform->setDateFormat(format);
}

void MicrosoftTimestampWidget::onNSChanged(QString nsString)
{
    bool ok = false;
    int ns = nsString.toInt(&ok);
    if (ok)
        transform->setOutNS(ns);
}

void MicrosoftTimestampWidget::localTzToggled(bool checked)
{
    if (checked) {
        transform->setTZ(MicrosoftTimestamp::TZ_LOCAL);
    } else {
        transform->setTZ(MicrosoftTimestamp::TZ_UTC);
    }
}
