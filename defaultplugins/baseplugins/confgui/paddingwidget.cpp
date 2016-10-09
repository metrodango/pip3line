/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "paddingwidget.h"
#include "ui_paddingwidget.h"
#include <QDebug>

PaddingWidget::PaddingWidget(Padding *ntransform, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::PaddingWidget();
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::PaddingWidget X{");
    }
    transform = ntransform;
    ui->setupUi(this);
    ui->blockSizeSpinBox->setMinimum(Padding::MINBLOCKSIZE);
    ui->blockSizeSpinBox->setMaximum(Padding::MAXBLOCKSIZE);
    ui->typeComboBox->setCurrentIndex((int)transform->getVariant());
    ui->blockSizeSpinBox->setValue(transform->getBlocksize());
    ui->charWidget->setChar(transform->getPadChar());

    connect(ui->typeComboBox,SIGNAL(currentIndexChanged(int)), this,SLOT(onTypeChange(int)));
    connect(ui->blockSizeSpinBox,SIGNAL(valueChanged(int)), this, SLOT(onBlockSizeChange(int)));
    connect(ui->charWidget,SIGNAL(charChanged(char)), this, SLOT(onPaddingCharChange(char)));
}

PaddingWidget::~PaddingWidget()
{
    delete ui;
}

void PaddingWidget::onTypeChange(int index) {
    transform->setVariant((Padding::PaddingVariant)index);
}

void PaddingWidget::onPaddingCharChange(char value) {
    transform->setPadChar(value);
    ui->typeComboBox->blockSignals(true);
    ui->typeComboBox->setCurrentIndex((int)Padding::CUSTOM);
    ui->typeComboBox->blockSignals(false);
}

void PaddingWidget::onBlockSizeChange(int size)
{
    transform->setBlockSize(size);
}
