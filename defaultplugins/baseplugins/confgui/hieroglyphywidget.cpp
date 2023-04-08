/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#include "hieroglyphywidget.h"
#include "ui_hieroglyphywidget.h"

HieroglyphyWidget::HieroglyphyWidget(Hieroglyphy * ntransform, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::HieroglyphyWidget();
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::HieroglyphyWidget X{");
    }
    transform = ntransform;
    ui->setupUi(this);

    ui->btoaCheckBox->setChecked(transform->isBtoaInUse());
}

HieroglyphyWidget::~HieroglyphyWidget()
{
    delete ui;
}

void HieroglyphyWidget::on_btoaCheckBox_clicked()
{
    transform->setUseBtoa(ui->btoaCheckBox->isChecked());
}
