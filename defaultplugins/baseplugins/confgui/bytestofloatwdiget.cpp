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
    if (ui == nullptr)
        qFatal("Cannot allocate memory for Ui::BytesToFloatWdiget X{");

    transform = ntransform;
    ui->setupUi(this);

    connect(ui->littleEndianRadioButton, &QRadioButton::toggled, this, [=](bool checked) {transform->setLittleEndian(checked);});
    connect(ui->float32radioButton, &QRadioButton::toggled, this, [=](bool checked) {
        transform->setFloatSize( checked ? BytesToFloat::F32bits : BytesToFloat::F64bits);
    });
    //connect(ui->precisionSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, [=](int val) { transform->setPrecision(val); });
    connect(ui->precisionSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onPrecisionChanged(int)));
}

BytesToFloatWdiget::~BytesToFloatWdiget()
{
    delete ui;
}

void BytesToFloatWdiget::onPrecisionChanged(int val)
{
    transform->setPrecision(val);
}
