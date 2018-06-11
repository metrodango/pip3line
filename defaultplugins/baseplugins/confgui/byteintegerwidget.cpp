/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "byteintegerwidget.h"
#include "ui_byteintegerwidget.h"

ByteIntegerWidget::ByteIntegerWidget(BytesInteger *ntransform, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::ByteIntegerWidget();
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::ByteIntegerWidget X{");
    }
    transform = ntransform;
    ui->setupUi(this);
    if (transform->isLittleEndian()) {
        ui->littleEndianRadioButton->setChecked(true);
    } else {
        ui->bigEndianRadioButton->setChecked(true);
    }

    if (transform->isSignedInteger()) {
        ui->signedRadioButton->setChecked(true);
        ui->unsignedRadioButton->setChecked(false);
    } else {
        ui->signedRadioButton->setChecked(false);
        ui->unsignedRadioButton->setChecked(true);
    }

    switch (transform->getIntegerSize()) {
    case BytesInteger::I8bits:
        ui->I8bitsRadioButton->setChecked(true);
        break;
    case BytesInteger::I16bits:
        ui->I16bitsRadioButton->setChecked(true);
        break;
    case BytesInteger::I32bits:
        ui->I32bitsRadioButton->setChecked(true);
        break;
    case BytesInteger::I64bits:
        ui->I64bitsRadioButton->setChecked(true);
        break;
    }

    connect(ui->signedRadioButton, &QRadioButton::toggled, this, [=](bool val) { transform->setSignedInteger(val);});
    connect(ui->littleEndianRadioButton, &QRadioButton::toggled, this, [=](bool val) { transform->setLittleEndian(val);});
    connect(ui->I8bitsRadioButton, &QRadioButton::clicked, this, &ByteIntegerWidget::sizeChanged);
    connect(ui->I16bitsRadioButton, &QRadioButton::clicked, this, &ByteIntegerWidget::sizeChanged);
    connect(ui->I32bitsRadioButton, &QRadioButton::clicked, this, &ByteIntegerWidget::sizeChanged);
    connect(ui->I64bitsRadioButton, &QRadioButton::clicked, this, &ByteIntegerWidget::sizeChanged);
}

ByteIntegerWidget::~ByteIntegerWidget()
{
    delete ui;
}

void ByteIntegerWidget::sizeChanged()
{
    if (ui->I8bitsRadioButton->isChecked())
        transform->setIntegerSize(BytesInteger::I8bits);
    else if (ui->I16bitsRadioButton->isChecked())
        transform->setIntegerSize(BytesInteger::I16bits);
    else if (ui->I32bitsRadioButton->isChecked())
        transform->setIntegerSize(BytesInteger::I32bits);
    else if (ui->I64bitsRadioButton->isChecked())
        transform->setIntegerSize(BytesInteger::I64bits);
}
