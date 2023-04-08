/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
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
    connect(ui->formatLineEdit, &QLineEdit::textChanged, this, [=](const QString & format) {
        transform->setDateFormat(format);
    });
    connect(ui->tzLocalRadioButton, &QRadioButton::toggled, this, [=](bool checked){
        transform->setTZ(checked ? TimeStamp::TZ_LOCAL : TimeStamp::TZ_UTC);
    });

}

TimestampWidget::~TimestampWidget()
{
    delete ui;
}
