/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#include "newbytedialog.h"
#include "ui_newbytedialog.h"

NewByteDialog::NewByteDialog(GuiHelper * guiHelper, QWidget *parent, bool isSimple) :
    AppDialog(guiHelper, parent)
{
    ui = new(std::nothrow) Ui::NewByteDialog();
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::NewByteDialog X{");
    }
    ui->setupUi(this);
    if (isSimple) {
        ui->countLabel->setVisible(false);
        ui->countSpinBox->setVisible(false);
    }
}

NewByteDialog::~NewByteDialog()
{
    delete ui;
}

int NewByteDialog::byteCount()
{
    return ui->countSpinBox->value();
}

char NewByteDialog::getChar()
{
    return ui->hexWidget->getChar();
}

void NewByteDialog::closeEvent(QCloseEvent *event)
{
    reject();
    QDialog::closeEvent(event);
}
