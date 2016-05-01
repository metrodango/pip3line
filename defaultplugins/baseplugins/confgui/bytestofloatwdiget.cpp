/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "bytestofloatwdiget.h"
#include "ui_bytestofloatwdiget.h"

BytesToFloatWdiget::BytesToFloatWdiget(BytesToFloat *ntransform, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::BytesToFloatWdiget();
    if (ui == NULL)
        qFatal("Cannot allocate memory for Ui::BytesToFloatWdiget X{");

    transform = ntransform;
    ui->setupUi(this);

    connect(ui->littleEndianRadioButton, SIGNAL(toggled(bool)), this, SLOT(onLittleEndianChange(bool)));
    connect(ui->float32radioButton, SIGNAL(toggled(bool)), this, SLOT(on32BitSizeChanged(bool)));
    connect(ui->precisionSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onPrecisionChanged(int)));
}

BytesToFloatWdiget::~BytesToFloatWdiget()
{
    delete ui;
}

void BytesToFloatWdiget::onLittleEndianChange(bool checked)
{
    transform->setLittleEndian(checked);
}

void BytesToFloatWdiget::on32BitSizeChanged(bool checked)
{
    if (checked) {
        transform->setFloatSize(BytesToFloat::F32bits);
    } else {
        transform->setFloatSize(BytesToFloat::F64bits);
    }
}

void BytesToFloatWdiget::onPrecisionChanged(int val)
{
    transform->setPrecision(val);
}
