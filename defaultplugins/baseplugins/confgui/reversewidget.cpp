/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#include "reversewidget.h"
#include "ui_reversewidget.h"

ReverseWidget::ReverseWidget(Reverse *ntransform, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::ReverseWidget();
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::ReverseWidget X{");
    }
    transform = ntransform;
    ui->setupUi(this);
    ui->blockSizeSpinBox->setValue(transform->getBlocksize());
    ui->blockSizeSpinBox->setMinimum(Reverse::MINBLOCKSIZE);
    ui->blockSizeSpinBox->setMaximum(Reverse::MAXBLOCKSIZE);
    ui->entireCheckBox->setChecked(transform->getNoBlock());

    connect(ui->entireCheckBox, &QCheckBox::toggled,this, &ReverseWidget::onEntireCheckBoxChange);
    //connect(ui->blockSizeSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, &ReverseWidget::onBlockSizeChange);
    connect(ui->blockSizeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onBlockSizeChange(int)));
}

ReverseWidget::~ReverseWidget()
{
    delete ui;
}

void  ReverseWidget::onBlockSizeChange(int value) {
    ui->blockSizeSpinBox->blockSignals(true);
    if (!transform->setBlocksize(value)) {
        if (value < Reverse::MINBLOCKSIZE) {
            ui->blockSizeSpinBox->setValue(Reverse::MINBLOCKSIZE);
        } else {
            ui->blockSizeSpinBox->setValue(Reverse::MAXBLOCKSIZE);
        }
    } else {
        ui->entireCheckBox->blockSignals(true);
        ui->entireCheckBox->setChecked(false);
        ui->entireCheckBox->blockSignals(false);
    }
    ui->blockSizeSpinBox->blockSignals(false);
}

void  ReverseWidget::onEntireCheckBoxChange(bool checked) {
    transform->setNoBlock(checked);
}
