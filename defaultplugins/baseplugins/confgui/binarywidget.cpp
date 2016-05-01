/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "binarywidget.h"
#include "ui_binarywidget.h"

BinaryWidget::BinaryWidget(Binary *ntransform, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::BinaryWidget();
    if (ui == NULL) {
        qFatal("Cannot allocate memory for Ui::BinaryWidget X{");
    }
    transform = ntransform;
    ui->setupUi(this);

    ui->groupBySpinBox->setValue(transform->getBlockSize());
    ui->groupBySpinBox->setMinimum(Binary::MINBLOCKSIZE);
    ui->groupBySpinBox->setMaximum(Binary::MAXBLOCKSIZE);
    connect(ui->groupBySpinBox,SIGNAL(valueChanged(int)), this, SLOT(onChangeGroupBy(int)));
}

BinaryWidget::~BinaryWidget()
{
    delete ui;
}

void BinaryWidget::onChangeGroupBy(int value)
{
    ui->groupBySpinBox->blockSignals(true);
    if (!transform->setBlockSize(value)) {
        if (value < Binary::MINBLOCKSIZE) {
            ui->groupBySpinBox->setValue(Binary::MINBLOCKSIZE);
        } else {
            ui->groupBySpinBox->setValue(Binary::MAXBLOCKSIZE);
        }
    }
    ui->groupBySpinBox->blockSignals(false);
}
