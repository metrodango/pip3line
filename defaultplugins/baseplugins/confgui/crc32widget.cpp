/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "crc32widget.h"
#include "ui_crc32widget.h"

Crc32Widget::Crc32Widget(Crc32 *ntransform, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::Crc32Widget();
    if (ui == NULL) {
        qFatal("Cannot allocate memory for Ui::Crc32Widget X{");
    }
    transform = ntransform;
    ui->setupUi(this);

    ui->littleendianRadioButton->setChecked(transform->isLittleendian());
    ui->hexaRadioButton->setChecked(transform->isHexOutput());
    ui->appendCheckBox->setChecked(transform->getAppendToInput());
    connect(ui->littleendianRadioButton, SIGNAL(toggled(bool)), SLOT(onLittleendianToggled(bool)));
    connect(ui->hexaRadioButton, SIGNAL(toggled(bool)), SLOT(onHexaRadioButtonToggled(bool)));
    connect(ui->appendCheckBox, SIGNAL(toggled(bool)), SLOT(onAppendCheckBoxToggled(bool)));
}

Crc32Widget::~Crc32Widget()
{
    delete ui;
}

void Crc32Widget::onLittleendianToggled(bool checked)
{
    transform->setLittleendian(checked);
}

void Crc32Widget::onHexaRadioButtonToggled(bool checked)
{
    transform->setHexOutput(checked);
}

void Crc32Widget::onAppendCheckBoxToggled(bool checked)
{
    transform->setAppendToInput(checked);
}
