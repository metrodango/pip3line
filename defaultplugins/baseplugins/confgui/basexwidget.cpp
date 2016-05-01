/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "basexwidget.h"
#include "ui_basexwidget.h"

BaseXWidget::BaseXWidget(BaseX *ntransform, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::BaseXWidget();
    if (ui == NULL) {
        qFatal("Cannot allocate memory for Ui::BaseXWidget X{");
    }
    transform = ntransform;
    ui->setupUi(this);
    ui->baseSpinBox->setValue(transform->getBase());
    ui->uppercaseCheckBox->setChecked(transform->getUppercase());
    connect(ui->baseSpinBox,SIGNAL(valueChanged(int)), this, SLOT(onBaseChange(int)));
    connect(ui->uppercaseCheckBox,SIGNAL(toggled(bool)), this, SLOT(onUpperCaseChange(bool)));
}

BaseXWidget::~BaseXWidget()
{
    delete ui;
}

void BaseXWidget::onBaseChange(int val)
{
    transform->setBase(val);
}

void BaseXWidget::onUpperCaseChange(bool val)
{
    transform->setUppercase(val);
}
