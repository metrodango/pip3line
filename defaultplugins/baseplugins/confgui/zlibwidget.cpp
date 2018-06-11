/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "zlibwidget.h"
#include "ui_zlibwidget.h"


ZlibWidget::ZlibWidget(Zlib *ntransform, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::ZlibWidget();
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::ZlibWidget X{");
    }
    transform = ntransform;
    ui->setupUi(this);
    ui->compressionSpinBox->setValue(transform->getCompression());
    ui->removeHeaderCheckBox->setChecked(transform->doRemoveHeader());

    connect(ui->removeHeaderCheckBox, &QCheckBox::toggled, this, [=](bool checked) {
        transform->setRemoveHeader(checked);
    });

    //connect(ui->compressionSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, &ZlibWidget::onCompressionChanged);
    connect(ui->compressionSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onCompressionChanged(int)));
}

ZlibWidget::~ZlibWidget()
{
    delete ui;
}

void ZlibWidget::onCompressionChanged(int value)
{
    if (!transform->setCompression(value)) {
        ui->compressionSpinBox->setValue(transform->getCompression());
    }
}
