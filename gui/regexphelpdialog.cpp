/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#include "regexphelpdialog.h"
#include "ui_regexphelpdialog.h"

RegExpHelpDialog::RegExpHelpDialog(GuiHelper * nguiHelper,QWidget *parent) :
    AppDialog(nguiHelper,parent)
{
    ui = new(std::nothrow) Ui::RegExpHelpDialog();
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::RegExpHelpDialog X{");
    }
    ui->setupUi(this);
}

RegExpHelpDialog::~RegExpHelpDialog()
{
    delete ui;
}
