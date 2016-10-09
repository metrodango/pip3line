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
    connect(ui->hexadecimalRadioButton, SIGNAL(toggled(bool)), this, SLOT(baseChanged()));
    connect(ui->decimalRadioButton, SIGNAL(toggled(bool)), this, SLOT(baseChanged()));

    connect(ui->littleEndianRadioButton, SIGNAL(toggled(bool)), this, SLOT(endianChanged()));
    connect(ui->bigendianRadioButton, SIGNAL(toggled(bool)), this, SLOT(endianChanged()));

    connect(ui->upperCaseRadioButton, SIGNAL(toggled(bool)), this, SLOT(caseChanged()));
    connect(ui->lowerCaseRadioButton, SIGNAL(toggled(bool)), this, SLOT(caseChanged()));
}

IPTranslateIPv4Widget::~IPTranslateIPv4Widget()
{
    delete ui;
}

void IPTranslateIPv4Widget::endianChanged()
{
    if (ui->littleEndianRadioButton->isChecked())
        transform->setLittleEndian(true);
    else
        transform->setLittleEndian(false);
}

void IPTranslateIPv4Widget::caseChanged()
{
    if (ui->upperCaseRadioButton->isChecked())
        transform->setUppercase(true);
    else
        transform->setUppercase(false);
}

void IPTranslateIPv4Widget::baseChanged()
{
    if (ui->hexadecimalRadioButton->isChecked())
        transform->setBase(IPTranslateIPv4::BASE16);
    else
        transform->setBase(IPTranslateIPv4::BASE10);
}
