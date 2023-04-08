/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#include "crc32widget.h"
#include "ui_crc32widget.h"

Crc32Widget::Crc32Widget(Crc32 *ntransform, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::Crc32Widget();
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::Crc32Widget X{");
    }
    transform = ntransform;
    ui->setupUi(this);

    ui->littleendianRadioButton->setChecked(transform->isLittleendian());
    ui->hexaRadioButton->setChecked(transform->isHexOutput());
    ui->appendCheckBox->setChecked(transform->getAppendToInput());
    connect(ui->littleendianRadioButton, &QRadioButton::toggled, this, [=](bool checked){transform->setLittleendian(checked);});
    connect(ui->hexaRadioButton, &QRadioButton::toggled, this, [=](bool checked) {transform->setHexOutput(checked);});
    connect(ui->appendCheckBox, &QCheckBox::toggled, this, [=](bool checked) {transform->setAppendToInput(checked);});
}

Crc32Widget::~Crc32Widget()
{
    delete ui;
}
