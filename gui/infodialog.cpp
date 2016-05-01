/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "infodialog.h"
#include <transformabstract.h>
#include "ui_infodialog.h"

InfoDialog::InfoDialog(GuiHelper * guiHelper, TransformAbstract *ntransform, QWidget *parent) :
    AppDialog(guiHelper, parent)
{
    ui = new(std::nothrow) Ui::InfoDialog();
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::InfoDialog X{");
    }
    transform = ntransform;
    ui->setupUi(this);

    ui->helpTextEdit->setHtml(transform->help());
    if (!transform->credits().isEmpty()) {
        ui->creditsTextEdit->setHtml(transform->credits());
    } else {
        ui->creditsTab->setVisible(false);
    }
}

InfoDialog::~InfoDialog()
{
    delete ui;
}

TransformAbstract *InfoDialog::getTransform()
{
    return transform;
}
