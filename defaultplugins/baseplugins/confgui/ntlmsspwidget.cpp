/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#include "ntlmsspwidget.h"
#include "ui_ntlmsspwidget.h"

NtlmsspWidget::NtlmsspWidget(Ntlmssp *ntransform, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::NtlmsspWidget();
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::NtlmsspWidget X{");
    }
    transform = ntransform;
    ui->setupUi(this);
    ui->decodeCheckBox->setChecked(transform->decodeBase64());

    connect(ui->decodeCheckBox, &QCheckBox::toggled, this, [=](bool checked){transform->setDecodeBase64(checked);});
}

NtlmsspWidget::~NtlmsspWidget()
{
    delete ui;
}
