/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "iptranslateipv4widget.h"
#include "ui_iptranslateipv4widget.h"
#include <QDebug>

IPTranslateIPv4Widget::IPTranslateIPv4Widget(IPTranslateIPv4 *ntransform, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::IPTranslateIPv4Widget();
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::IPTranslateIPv4Widget X{");
    }
    transform = ntransform;
    ui->setupUi(this);

    if (transform->isLittleendian()) {
        ui->littleEndianRadioButton->setChecked(true);
    } else {
        ui->bigendianRadioButton->setChecked(true);
    }

    if (transform->isFormatUpperCase()) {
        ui->upperCaseRadioButton->setChecked(true);
    } else {
        ui->lowerCaseRadioButton->setChecked(true);
    }

    if (transform->getBase() == IPTranslateIPv4::BASE16) {
        ui->hexadecimalRadioButton->setChecked(true);
    } else {
        ui->decimalRadioButton->setChecked(true);
    }
    connect(ui->hexadecimalRadioButton, &QRadioButton::toggled, this, [=](bool checked){transform->setBase(checked ? IPTranslateIPv4::BASE16 : IPTranslateIPv4::BASE10);});
    connect(ui->littleEndianRadioButton, &QRadioButton::toggled, this, [=](bool checked){transform->setLittleEndian(checked);});
    connect(ui->upperCaseRadioButton, &QRadioButton::toggled, this, [=](bool checked){transform->setUppercase(checked);});
}

IPTranslateIPv4Widget::~IPTranslateIPv4Widget()
{
    delete ui;
}
