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
    ui->typeComboBox->setCurrentIndex(static_cast<int>(transform->getVariant()));
    ui->blockSizeSpinBox->setValue(transform->getBlocksize());
    ui->charWidget->setChar(transform->getPadChar());

    //connect(ui->typeComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int index){transform->setVariant((Padding::PaddingVariant)index);});
    connect(ui->typeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onVariantChanged(int)));
    //connect(ui->blockSizeSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, [=](int size) {transform->setBlockSize(size);});
    connect(ui->blockSizeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onBlockSizeChanged(int)));
    connect(ui->charWidget, &HexWidget::charChanged, this, &PaddingWidget::onPaddingCharChange);
}

PaddingWidget::~PaddingWidget()
{
    delete ui;
}

void PaddingWidget::onVariantChanged(int val)
{
    transform->setVariant(static_cast<Padding::PaddingVariant>(val));
}

void PaddingWidget::onBlockSizeChanged(int val)
{
    transform->setBlockSize(val);
}

void PaddingWidget::onPaddingCharChange(char value) {
    transform->setPadChar(value);
    ui->typeComboBox->blockSignals(true);
    ui->typeComboBox->setCurrentIndex(static_cast<int>(Padding::CUSTOM));
    ui->typeComboBox->blockSignals(false);
}
