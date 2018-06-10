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
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::HtmlWidget X{");
    }
    transform = ntransform;
    ui->setupUi(this);

    ui->encodeAllRadioButton->setChecked(transform->doWeEncodeAll());
    ui->encoderOnlyRadioButton->setChecked(!transform->doWeEncodeAll());
    ui->hexaRadioButton->setChecked(transform->doWeUseHexadecimal());
    ui->decimalRadioButton->setChecked(!transform->doWeUseHexadecimal());
    ui->entityNameCheckBox->setChecked(transform->doWeUseName());

    connect(ui->encodeAllRadioButton,&QRadioButton::toggled, this, [=](bool checked) {transform->setEncodeAll(checked);});
    connect(ui->hexaRadioButton, &QRadioButton::toggled, this, [=](bool checked) {transform->setUseHexadecimal(checked);});
    connect(ui->entityNameCheckBox, &QCheckBox::toggled, this, [=](bool checked) {transform->setUseName(checked);});
}

HtmlWidget::~HtmlWidget()
{
    delete ui;
}
