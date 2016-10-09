/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "timestampwidget.h"
#include "ui_timestampwidget.h"

TimestampWidget::TimestampWidget(TimeStamp *ntransform, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::TimestampWidget();
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::TimestampWidget X{");
    }
    transform = ntransform;
    ui->setupUi(this);

    ui->formatLineEdit->setText(transform->getDateFormat());
    ui->formatLineEdit->setReadOnly(true);
    if (transform->getTZ() == TimeStamp::TZ_UTC) {
        ui->tzUTCRadioButton->setChecked(true);
    } else {
        ui->tzLocalRadioButton->setChecked(true);
    }
    connect(ui->formatLineEdit, SIGNAL(textChanged(QString)), this, SLOT(formatChanged(QString)));
    connect(ui->tzLocalRadioButton, SIGNAL(toggled(bool)), this, SLOT(outBoundTZLocalChanged(bool)));

}

TimestampWidget::~TimestampWidget()
{
    delete ui;
}

void TimestampWidget::formatChanged(QString format)
{
    transform->setDateFormat(format);
}

void TimestampWidget::outBoundTZLocalChanged(bool checked)
{
    if (checked) {
        transform->setTZ(TimeStamp::TZ_LOCAL);
    } else {
        transform->setTZ(TimeStamp::TZ_UTC);
    }
}
