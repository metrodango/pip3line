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
    if (ui == NULL) {
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

    connect(ui->signedRadioButton, SIGNAL(toggled(bool)), this, SLOT(signedChanged(bool)));
    connect(ui->littleEndianRadioButton, SIGNAL(toggled(bool)), this, SLOT(bytesOrderChanged(bool)));
    connect(ui->I8bitsRadioButton, SIGNAL(clicked()), this, SLOT(sizeChanged()));
    connect(ui->I16bitsRadioButton, SIGNAL(clicked()), this, SLOT(sizeChanged()));
    connect(ui->I32bitsRadioButton, SIGNAL(clicked()), this, SLOT(sizeChanged()));
    connect(ui->I64bitsRadioButton, SIGNAL(clicked()), this, SLOT(sizeChanged()));
}

ByteIntegerWidget::~ByteIntegerWidget()
{
    delete ui;
}

void ByteIntegerWidget::signedChanged(bool val)
{
    transform->setSignedInteger(val);
}

void ByteIntegerWidget::bytesOrderChanged(bool val)
{
    transform->setLittleEndian(val);
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
