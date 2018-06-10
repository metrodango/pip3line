/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "rotxwidget.h"
#include "ui_rotxwidget.h"

RotXWidget::RotXWidget(Rotx *ntransform, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::RotXWidget();
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::RotXWidget X{");
    }
    transform = ntransform;
    ui->setupUi(this);
    ui->typeComboBox->setCurrentIndex(transform->getRotVariant());
    //connect(ui->typeComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int index){transform->setRotVariant((Rotx::RotVariant)index);});
    connect(ui->typeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onVariantChanged(int)));
}

RotXWidget::~RotXWidget()
{
    delete ui;
}

void RotXWidget::onVariantChanged(int index)
{
    transform->setRotVariant((Rotx::RotVariant)index);
}
