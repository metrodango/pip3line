/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "htmlwidget.h"
#include "ui_htmlwidget.h"

HtmlWidget::HtmlWidget(Html *ntransform, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::HtmlWidget();
    if (ui == NULL) {
        qFatal("Cannot allocate memory for Ui::HtmlWidget X{");
    }
    transform = ntransform;
    ui->setupUi(this);

    ui->encodeAllRadioButton->setChecked(transform->doWeEncodeAll());
    ui->encoderOnlyRadioButton->setChecked(!transform->doWeEncodeAll());
    ui->hexaRadioButton->setChecked(transform->doWeUseHexadecimal());
    ui->decimalRadioButton->setChecked(!transform->doWeUseHexadecimal());
    ui->entityNameCheckBox->setChecked(transform->doWeUseName());

    connect(ui->encodeAllRadioButton,SIGNAL(toggled(bool)), this, SLOT(onChangeScope()));
    connect(ui->encoderOnlyRadioButton,SIGNAL(toggled(bool)), this, SLOT(onChangeScope()));
    connect(ui->decimalRadioButton, SIGNAL(toggled(bool)), this, SLOT(onChangeType()));
    connect(ui->hexaRadioButton, SIGNAL(toggled(bool)), this, SLOT(onChangeType()));
    connect(ui->entityNameCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onChangeUseName()));
}

HtmlWidget::~HtmlWidget()
{
    delete ui;
}

void HtmlWidget::onChangeScope()
{
    transform->setEncodeAll(ui->encodeAllRadioButton->isChecked());
}

void HtmlWidget::onChangeType()
{
   transform->setUseHexadecimal(ui->hexaRadioButton->isChecked());
}

void HtmlWidget::onChangeUseName()
{
    transform->setUseName(ui->entityNameCheckBox->isChecked());
}
