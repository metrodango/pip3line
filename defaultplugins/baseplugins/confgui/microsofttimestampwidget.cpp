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
    connect(ui->formatLineEdit, &QLineEdit::textChanged, this, [=](const QString & format) {transform->setDateFormat(format);});
    connect(ui->nanoLineEdit, &QLineEdit::textChanged, this, &MicrosoftTimestampWidget::onNSChanged);
    connect(ui->localTimeRadioButton, &QRadioButton::toggled, this, [=](bool checked) {transform->setTZ(checked ? MicrosoftTimestamp::TZ_LOCAL : MicrosoftTimestamp::TZ_UTC);});
}

MicrosoftTimestampWidget::~MicrosoftTimestampWidget()
{
    delete ui;
}

void MicrosoftTimestampWidget::onNSChanged(QString nsString)
{
    bool ok = false;
    int ns = nsString.toInt(&ok);
    if (ok)
        transform->setOutNS(ns);
}
