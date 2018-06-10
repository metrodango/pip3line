/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "byterotwidget.h"
#include "ui_byterotwidget.h"
#include "../byterot.h"

ByteRotWidget::ByteRotWidget(ByteRot * transf, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::ByteRotWidget();
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::ByteRotWidget X{");
    }

    ui->setupUi(this);
    transform = transf;
    ui->spinBox->setValue(transform->getRotation());

    //connect(ui->spinBox, qOverload<int>(&QSpinBox::valueChanged), this, [=](int val) {transform->setRotation(val);});
    connect(ui->spinBox, SIGNAL(valueChanged(int)), this, SLOT(onRotationChanged(int)));
}

ByteRotWidget::~ByteRotWidget()
{
    delete ui;
}

void ByteRotWidget::onRotationChanged(int val)
{
    transform->setRotation(val);
}
